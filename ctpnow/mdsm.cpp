#include "mdsm.h"
#include "ThostFtdcMdApi.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "logger.h"
#include "ringbuffer.h"
#include "servicemgr.h"
#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QTimer>

///////////
class MdSmSpi : public CThostFtdcMdSpi {
public:
    explicit MdSmSpi(MdSm* sm)
        : sm_(sm)
    {
    }

private:
    void OnFrontConnected() override
    {
        info(__FUNCTION__);
        emit sm()->statusChanged(MDSM_CONNECTED);
    }

    // 如果网络异常，会直接调用OnFrontDisconnected，需要重置状态数据=
    // 网络错误当再次恢复时候，会自动重连重新走OnFrontConnected
    void OnFrontDisconnected(int nReason) override
    {
        info(QString().sprintf("MdSmSpi::OnFrontDisconnected,nReason=0x%x",
            nReason));

        resetData();
        emit sm()->statusChanged(MDSM_DISCONNECTED);
    }

    // errorId=7，msg=CTP:还没有初始化=
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                emit sm()->statusChanged(MDSM_LOGINFAIL);
            } else {
                emit sm()->statusChanged(MDSM_LOGINED);
            }
        }
    }

    // logout在tdapi里面是有效的,mdapi无效不用处理=
    void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override {}

    void OnRspError(CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override
    {
        if (bIsLast) {
            info(__FUNCTION__);
            isErrorRsp(pRspInfo, nRequestID);
        }
    }

    // 订阅成功了也会调用,目前是不管啥都返回订阅成功=
    void OnRspSubMarketData(
        CThostFtdcSpecificInstrumentField* pSpecificInstrument,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override
    {
        if (!isErrorRsp(pRspInfo, nRequestID) && pSpecificInstrument) {
            QString iid = pSpecificInstrument->InstrumentID;
            got_ids_ << iid;
        }

        if (bIsLast && got_ids_.length()) {
            QString ids;
            for (auto id : got_ids_) {
                ids = ids + id + ";";
            }
            info(QString().sprintf("total sub ids:%d,%s", got_ids_.length(),
                ids.toUtf8().constData()));
        }
    }

    void OnRspUnSubMarketData(
        CThostFtdcSpecificInstrumentField* pSpecificInstrument,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override {}

    void OnRtnDepthMarketData(
        CThostFtdcDepthMarketDataField* pDepthMarketData) override
    {
        QString id = pDepthMarketData->InstrumentID;
        auto rb = getRingBuffer(id);
        if (!isValidTick(rb, pDepthMarketData)) {
            return;
        }
        void* newTick = rb->put(pDepthMarketData);
        emit sm()->gotTick(newTick);
    }

private:
    bool isErrorRsp(CThostFtdcRspInfoField* pRspInfo, int reqId)
    {
        if (pRspInfo && pRspInfo->ErrorID != 0) {
            info(QString().sprintf(
                "<==错误，reqid=%d,errorId=%d，msg=%s", reqId, pRspInfo->ErrorID,
                gbk2utf16(pRspInfo->ErrorMsg).toUtf8().constData()));
            return true;
        }
        return false;
    }

    MdSm* sm() { return sm_; }

    void resetData()
    {
        got_ids_.clear();
        freeRingBuffer();

        // 发一个收盘信号，便于最后一分钟的数据写盘，做数据收盘处理等等=
        emit sm()->tradeClosed();
    }

    void info(QString msg) { g_sm->logger()->info(msg); }

    RingBuffer* getRingBuffer(QString id)
    {
        RingBuffer* rb = rbs_.value(id);
        if (rb == nullptr) {
            qFatal("rb == nullptr");
        }

        return rb;
    }

    void initRingBuffer(QStringList ids)
    {
        if (rbs_.count() != 0) {
            qFatal("rbs_.count() != 0");
        }

        for (auto id : ids) {
            RingBuffer* rb = new RingBuffer;
            rb->init(sizeof(CThostFtdcDepthMarketDataField), ringBufferLen_);
            rbs_.insert(id, rb);
        }
    }

    void freeRingBuffer()
    {
        auto rb_list = rbs_.values();
        for (int i = 0; i < rb_list.length(); i++) {
            RingBuffer* rb = rb_list.at(i);
            rb->free();
            delete rb;
        }
        rbs_.clear();
    }

    // 每次收盘/断网/崩溃/退出等等,都会清空ringbufer，需要重来一下下面的逻辑=
    // 1.总成交量为0的：无效=
    // 2.确定第一个有效tick：交易日期+时间，和当前时间偏离在3分钟之外：无效=
    // 3.确定后续tick是否有效：和上一个tick相比，总成交量不变的：无效=
    bool isValidTick(RingBuffer* rb, CThostFtdcDepthMarketDataField* curTick)
    {
        if (curTick->Volume == 0) {
            return false;
        }
        auto lastTick = (CThostFtdcDepthMarketDataField*)rb->get(rb->head());
        if (!lastTick) {
            QDateTime curDt = QDateTime::currentDateTime();
            QDateTime tickDt = QDateTime::fromString(QString().sprintf("%s %s.%3d", curTick->TradingDay, curTick->UpdateTime, curTick->UpdateMillisec), "yyyymmdd hh:mm:ss.zzz");
            qint64 delta = qAbs(curDt.msecsTo(tickDt));
            if (delta >= 3 * 60 * 1000) {
                return false;
            }
        } else {
            if (lastTick->Volume == curTick->Volume) {
                return false;
            }
        }
        return true;
    }

private:
    MdSm* sm_;
    QStringList got_ids_;

    QMap<QString, RingBuffer*> rbs_;
    const int ringBufferLen_ = 256;

    friend MdSm;
};

///////////
MdSm::MdSm(QObject* parent)
    : QObject(parent)
{
}

MdSm::~MdSm() {}

bool MdSm::init(QString userId,
    QString password,
    QString brokerId,
    QString frontMd,
    QString flowPathMd)
{
    userId_ = userId;
    password_ = password;
    brokerId_ = brokerId;
    frontMd_ = frontMd;
    flowPathMd_ = flowPathMd;

    // check
    if (userId_.length() == 0 || password_.length() == 0 || brokerId_.length() == 0 || frontMd_.length() == 0 || flowPathMd_.length() == 0) {
        return false;
    }
    return true;
}

void MdSm::start()
{
    info(__FUNCTION__);

    if (mdapi_ != nullptr) {
        qFatal("mdapi_!=nullptr");
        return;
    }

    QDir dir;
    dir.mkpath(flowPathMd_);
    mdapi_ = CThostFtdcMdApi::CreateFtdcMdApi(flowPathMd_.toStdString().c_str());
    mdspi_ = new MdSmSpi(this);
    mdapi_->RegisterSpi(mdspi_);
    mdapi_->RegisterFront((char*)qPrintable(frontMd_));
    mdapi_->Init();
}

void MdSm::stop()
{
    info(__FUNCTION__);

    if (mdapi_ == nullptr) {
        qFatal("mdapi_==nullptr");
        return;
    }

    mdapi_->RegisterSpi(nullptr);
    mdapi_->Release();

    mdapi_ = nullptr;
    delete mdspi_;
    mdspi_ = nullptr;
    emit this->statusChanged(MDSM_STOPPED);
}

void MdSm::info(QString msg)
{
    g_sm->logger()->info(msg);
}

QString MdSm::version()
{
    return CThostFtdcMdApi::GetApiVersion();
}

void MdSm::login(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {
        CThostFtdcReqUserLoginField req;
        memset(&req, 0, sizeof(req));
        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.UserID, userId_.toStdString().c_str(), sizeof(req.UserID) - 1);
        strncpy(req.Password, password_.toStdString().c_str(), sizeof(req.Password) - 1);
        int result = mdapi_->ReqUserLogin(&req, ++reqId_);
        info(QString().sprintf("CmdMdLogin,reqId=%d,result=%d", reqId_, result));
        //  被流控，一秒后重来=
        if (result == -3) {
            login(retryAfterMsec_, robotId);
        } else {
            //发单成功，发信号，<reqId,robotId>，便于上层跟踪=
            emit requestSent(reqId_, robotId);
        }
    });
}

void MdSm::subscrible(QStringList ids,
    unsigned int delayTick,
    QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {
        QList<std::string> std_ids;
        char** cids = new char*[ids.length()];
        for (int i = 0; i < ids.length(); i++) {
            std_ids.append(ids.at(i).toStdString());
            cids[i] = (char*)std_ids.at(i).c_str();
        }
        int result = mdapi_->SubscribeMarketData(cids, ids.length());
        delete[] cids;
        info(QString().sprintf("CmdMdSubscrible,result=%d", result));
        if (result == -3) {
            subscrible(ids, retryAfterMsec_, robotId);
        } else {
            mdspi_->initRingBuffer(ids);
        }
    });
}
