#include "datapump.h"
#include "ringbuffer.h"
#include "ThostFtdcUserApiStruct.h"
#include "servicemgr.h"
#include "logger.h"
#include <windows.h>
#include <QLoggingCategory>
#include "ctpmgr.h"
#include <QDir>
#include <leveldb/db.h>
#include "file_utils.h"
#include "profile.h"
#include "ctp_utils.h"
#include "dbservice.h"

DataPump::DataPump(QObject* parent)
    : QObject(parent)
{
}

void DataPump::init()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::CTP);

    QObject::connect(this, &DataPump::gotTick, g_sm->dbService(), &DbService::putTick);
}

//guithread的eventloop退了，不会处理dbthread的finish，这里应该等待线程退出，然后清理qthread对象
//对象属于哪个线程就在哪个线程上清理=
//db_backend_自己删除自己=
void DataPump::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::CTP);
}

//在spi线程上直接调用=
void DataPump::putTick(void* tick)
{
    int indexRb = -1;
    RingBuffer* rb = nullptr;

    if (shouldSkipTick(tick)) {
        return;
    }

    void* newTick = this->putTickToRingBuffer(tick, indexRb, rb);
    fixTickMs(newTick, indexRb, rb);

    emit this->gotTick(newTick, indexRb, rb);
}

// todo(sunwangme):IH在9:15:00时候出现了买一卖一价非常庞大应该是没有初始化的问题，需要处理=
bool DataPump::shouldSkipTick(void *tick){
    // 如果时间无效不保存，如有效区间[09:15:00-15:30:00)[21:00:00-23:30:00) [00:00:00-02:30:00)
    // 交给客户端去做更合理，mdsrv只负责收原始数据=
    if (0){
        char* timeTick = ((CThostFtdcDepthMarketDataField*)tick)->UpdateTime;
        bool valid = false;

        // 金融期货 IF 中金所=
        if (memcmp(timeTick, "09:15:00", 8) >= 0 && memcmp(timeTick, "15:30:00", 8) < 0) {
            valid = true;
        };

        // 商品期货 SR 郑商所=
        if (memcmp(timeTick, "09:00:00", 8) >= 0 && memcmp(timeTick, "15:00:00", 8) < 0) {
            valid = true;
        };
        if (memcmp(timeTick, "21:00:00", 8) >= 0 && memcmp(timeTick, "23:30:00", 8) < 0) {
            valid = true;
        };

        // 商品期货 AG 上期所=
        if (memcmp(timeTick, "09:00:00", 8) >= 0 && memcmp(timeTick, "15:00:00", 8) < 0) {
            valid = true;
        };
        if (memcmp(timeTick, "21:00:00", 8) >= 0 && memcmp(timeTick, "23:59:59", 8) <= 0) {
            valid = true;
        };
        if (memcmp(timeTick, "00:00:00", 8) >= 0 && memcmp(timeTick, "02:30:00", 8) < 0) {
            valid = true;
        };

        if (!valid) {
            return true;
        }
    }

    // 如果和前一个tick一样就不保存了（时间，最新价，成交量，持仓量，买一卖一价，买一卖一申报量）
    //白糖会在每次开盘时候，先发一个上次的收盘tick但日期是不一样的，晕。如23号早上9:00:00会
    //收到一个22号的23:29:59的tick但日期却是23号=
    //todo(sunwangme):这么过滤也有问题，因为有可能晚上最后一根是23:29:58,早上发一个23:29:59
    //过来，按当前时间来过滤是最好的=
    if(1){
        CThostFtdcDepthMarketDataField* mdf = (CThostFtdcDepthMarketDataField*)tick;
        QString id = mdf->InstrumentID;
        RingBuffer* rb = getRingBuffer(id);
        CThostFtdcDepthMarketDataField* lastMdf = (CThostFtdcDepthMarketDataField*)rb->get(rb->head());
        if (lastMdf &&
                (memcmp(mdf->UpdateTime, lastMdf->UpdateTime, sizeof(mdf->UpdateTime) - 1) == 0) &&
                (mdf->LastPrice == lastMdf->LastPrice) &&
                (mdf->Volume == lastMdf->Volume) &&
                (mdf->OpenInterest == lastMdf->OpenInterest) &&
                (mdf->BidPrice1 == lastMdf->BidPrice1) &&
                (mdf->BidVolume1 == lastMdf->BidVolume1) &&
                (mdf->AskPrice1 == lastMdf->AskPrice1) &&
                (mdf->AskVolume1 == lastMdf->AskVolume1)) {
            return true;
        }
    }

    // 如果成交量 买一卖一价，买一卖一申报量 都为0，丢弃
    // 白糖会在夜盘之前发一个这样的tick
    // 发现ic会推一个买一卖一为DBL_MAX的东西过来=
    if(1){
        CThostFtdcDepthMarketDataField* mdf = (CThostFtdcDepthMarketDataField*)tick;
        if ( (mdf->Volume == 0) &&
             (!qIsFinite(mdf->BidPrice1) || mdf->BidPrice1 == 0.0 || mdf->BidPrice1 == DBL_MAX) &&
             (mdf->BidVolume1 == 0) &&
             (!qIsFinite(mdf->AskPrice1) || mdf->AskPrice1 == 0.0 || mdf->AskPrice1 == DBL_MAX) &&
             (mdf->AskVolume1 == 0) ){
            return true;
        }
    }

    return false;
}

void DataPump::putInstrument(void* pInstrument)
{
    g_sm->dbService()->putInstrument(pInstrument);
}

void DataPump::initRingBuffer(QStringList ids)
{
    if (rbs_.count() != 0) {
        qFatal("rbs_.count() != 0");
    }

    for (auto id : ids) {
        RingBuffer* rb = new RingBuffer;
        rb->init(sizeof(CThostFtdcDepthMarketDataField), ringBufferLen_);
        rbs_.insert(id, rb);
    }

    loadRingBufferFromBackend(ids);
}

//修补tick的ms还是需要回读一下=
void DataPump::loadRingBufferFromBackend(QStringList ids)
{
    auto db = g_sm->dbService()->getTickDB();
    for (auto id : ids) {
        auto rb = getRingBuffer(id);

        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        //第一个是id+
        //最后一个是id=
        QString key;
        key = id + QStringLiteral("=");
        it->Seek(leveldb::Slice(key.toStdString()));
        if (it->Valid()) {
            it->Prev();
        }
        int count = 0;
        for (; it->Valid() && count < rb->count(); it->Prev()) {
            //遇到了前后两个结束item
            auto mdf = (CThostFtdcDepthMarketDataField*)it->value().data();
            if (mdf->InstrumentID[0] == 0) {
                break;
            }
            count++;
            if (it->value().size() != sizeof(CThostFtdcDepthMarketDataField)) {
                qFatal("it->value().size() != sizeof(DepthMarketDataField)");
            }
            rb->load(rb->count() - count, mdf);
        }
        delete it;
    }
}

void DataPump::freeRingBuffer()
{
    auto rb_list = rbs_.values();
    for (int i = 0; i < rb_list.length(); i++) {
        RingBuffer* rb = rb_list.at(i);
        rb->free();
        delete rb;
    }
    rbs_.clear();
}

void* DataPump::putTickToRingBuffer(void* tick, int& index, RingBuffer*& rb)
{
    CThostFtdcDepthMarketDataField* mdf = (CThostFtdcDepthMarketDataField*)tick;
    QString id = mdf->InstrumentID;
    rb = getRingBuffer(id);
    return rb->put(tick, index);
}

RingBuffer* DataPump::getRingBuffer(QString id)
{
    RingBuffer* rb = rbs_.value(id);
    if (rb == nullptr) {
        qFatal("rb == nullptr");
    }

    return rb;
}

// 和前一个tick比较，如果time相同，就改ms为前一个的ms+1，不同，ms改为0
void DataPump::fixTickMs(void* tick, int indexRb, RingBuffer* rb)
{
    CThostFtdcDepthMarketDataField* preTick = nullptr;
    CThostFtdcDepthMarketDataField* curTick = (CThostFtdcDepthMarketDataField*)tick;
    int index = indexRb - 1;
    if (index < 0) {
        index = index + rb->count();
    }
    preTick = (CThostFtdcDepthMarketDataField*)rb->get(index);
    if (preTick && strcmp(curTick->UpdateTime, preTick->UpdateTime) == 0) {
        curTick->UpdateMillisec = preTick->UpdateMillisec + 1;
    }
    else {
        curTick->UpdateMillisec = 0;
    }
}

//初始化instrument定位=
void DataPump::initInstrumentLocator()
{
    g_sm->dbService()->initInstrumentLocator();
}

//初始化tick定位=
void DataPump::initTickLocator(QString id)
{
    g_sm->dbService()->initTickLocator(id);
}
