#include "dbservice.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/write_batch.h"
#include "profile.h"
#include "servicemgr.h"

DbService::DbService(QObject* parent)
    : QObject(parent)
{
}

void DbService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // init env
    leveldb::Env::Default();
    leveldb::BytewiseComparator();

    // dbOpen
    dbOpen();

    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTick, this, &DbService::onGotTick);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotContracts, this, &DbService::onGotContracts);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &DbService::onTradeWillBegin);
}

void DbService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // 把缓存的tick写了=
    batchWriteTicks();

    // dbClose
    dbClose();

    // free env
    delete leveldb::BytewiseComparator();
    delete leveldb::Env::Default();
}

leveldb::DB* DbService::getDb()
{
    BfDebug(__FUNCTION__);

    if (!db_) {
        qFatal("db not open yet");
        return nullptr;
    }

    return db_;
}

void DbService::dbOpen()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_) {
        BfInfo("db opened already");
        return;
    }

    QString path = Profile::dbPath();
    mkDir(path);
    leveldb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    options.compression = leveldb::kNoCompression;
    options.paranoid_checks = false;
    leveldb::DB* db;
    leveldb::Status status = leveldb::DB::Open(options,
        path.toStdString(),
        &db);
    if (!status.ok()) {
        qFatal("leveldb::DB::Open fail");
    }

    db_ = db;
}

void DbService::dbClose()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_ == nullptr) {
        BfInfo("db not open yet");
        return;
    }
    delete db_;
    db_ = nullptr;
}

void DbService::batchWriteTicks()
{
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (tickCount_ == 0) {
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;
    for (int i = 0; i < tickCount_; i++) {
        void* curTick = tickArray[i].curTick;
        void* preTick = tickArray[i].preTick;

        BfTickData bfItem;
        CtpUtils::translateTick(curTick, preTick, &bfItem);

        // tick里面的exchange不一定有=
        QString exchange = bfItem.exchange().c_str();
        if (exchange.trimmed().length() == 0) {
            void* contract = g_sm->ctpMgr()->getContract(bfItem.symbol().c_str());
            exchange = CtpUtils::getExchangeFromContract(contract);
            bfItem.set_exchange(exchange.toStdString());
        }

        // key: tick-symbol-exchange-actiondata-ticktime
        std::string key = QString().sprintf("tick-%s-%s-%s-%s",  bfItem.symbol().c_str(),bfItem.exchange().c_str(), bfItem.actiondate().c_str(), bfItem.ticktime().c_str()).toStdString();
        std::string val;
        bool ok = bfItem.SerializeToString(&val);
        if (!ok) {
            qFatal("SerializeToString fail");
        }

        batch.Put(key, val);
    }
    db_->Write(options, &batch);

    tickCount_ = 0;
}

// 128个tick写一次=
void DbService::onGotTick(void* curTick, void* preTick)
{
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (tickCount_ >= tickArrayLen_) {
        qFatal("tickCount_ >= tickArrayLen_");
        return;
    }

    tickArray[tickCount_].curTick = curTick;
    tickArray[tickCount_].preTick = preTick;
    tickCount_++;
    if (tickCount_ == tickArrayLen_) {
        batchWriteTicks();
    }
}

void DbService::onGotContracts(QStringList ids, QStringList idsAll)
{
    g_sm->checkCurrentOn(ServiceMgr::DB);

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;

    //按排序后合约来=
    //QStringList sorted_ids = idsAll;
    QStringList sorted_ids = ids;
    sorted_ids.sort();

    // key: contract+
    // key: contract=
    BfContractData bfNullContract;
    std::string key = "contract+";
    std::string val = bfNullContract.SerializeAsString();
    batch.Put(key, val);
    key = "contract=";
    val = bfNullContract.SerializeAsString();
    batch.Put(key, val);

    for (int i = 0; i < sorted_ids.length(); i++) {
        QString id = sorted_ids.at(i);
        void* contract = g_sm->ctpMgr()->getContract(id);
        BfContractData bfItem;
        CtpUtils::translateContract(contract, &bfItem);

        // key: contract-symbol-exchange
        key = QString().sprintf("contract-%s-%s",bfItem.symbol().c_str(),bfItem.exchange().c_str()).toStdString();
        bool ok = bfItem.SerializeToString(&val);
        if (!ok) {
            qFatal("SerializeToString fail");
        }
        batch.Put(key, val);

        // key: tick-symbol-exchange+
        // key: tick-symbol-exchange=
        BfTickData bfNullTick;
        key = QString().sprintf("tick-%s-%s+", bfItem.symbol().c_str(),bfItem.exchange().c_str()).toStdString();
        val = bfNullTick.SerializeAsString();
        batch.Put(key, val);
        key = QString().sprintf("tick-%s-%s=", bfItem.symbol().c_str(),bfItem.exchange().c_str()).toStdString();
        val = bfNullTick.SerializeAsString();
        batch.Put(key, val);
    }

    db_->Write(options, &batch);
}

void DbService::onTradeWillBegin()
{
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // 把缓存的tick写了=
    batchWriteTicks();
}
