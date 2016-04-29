#include "dbservice.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/write_batch.h"
#include "profile.h"
#include "proto_utils.h"
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

    // dbInit
    dbInit();
}

void DbService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

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

void DbService::dbInit()
{
    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;

    // key: contract+
    // key: contract=
    BfContractData bfNullContract;
    std::string key = "contract+";
    std::string val = bfNullContract.SerializeAsString();
    batch.Put(key, val);
    key = "contract=";
    val = bfNullContract.SerializeAsString();
    batch.Put(key, val);

    db_->Write(options, &batch);
}

void DbService::getTick(const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer)
{
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void DbService::getBar(const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer)
{
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void DbService::getContract(const BfDatafeedGetContractReq* request, ::grpc::ServerWriter<BfContractData>* writer)
{
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void DbService::insertTick(const BfTickData& bfItem)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (bfItem.symbol().length() == 0 || bfItem.exchange().length() == 0 || bfItem.actiondate().length() == 0 || bfItem.ticktime().length() == 0) {
        BfDebug("invalid tick,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;
    std::string key, val;

    if (1) {
        // key: tick-symbol-exchange-actiondata-ticktime
        key = QString().sprintf("tick-%s-%s-%s-%s", bfItem.symbol().c_str(), bfItem.exchange().c_str(), bfItem.actiondate().c_str(), bfItem.ticktime().c_str()).toStdString();
        bool ok = bfItem.SerializeToString(&val);
        if (!ok) {
            qFatal("SerializeToString fail");
        }
        batch.Put(key, val);
    }

    db_->Write(options, &batch);
}

void DbService::insertBar(const BfBarData& bfItem)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (bfItem.symbol().length() == 0 || bfItem.exchange().length() == 0 || bfItem.actiondate().length() == 0 || bfItem.bartime().length() == 0) {
        BfDebug("invalid bar,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;
    std::string key, val;

    if (1) {
        // key: bar-symbol-exchange-period-actiondata-bartime
        key = QString().sprintf("bar-%s-%s-%s-%s-%s", bfItem.symbol().c_str(), bfItem.exchange().c_str(), qPrintable(ProtoUtils::formatPeriod(bfItem.period())), bfItem.actiondate().c_str(), bfItem.bartime().c_str()).toStdString();
        bool ok = bfItem.SerializeToString(&val);
        if (!ok) {
            qFatal("SerializeToString fail");
        }
        batch.Put(key, val);
    }

    db_->Write(options, &batch);
}

void DbService::insertContract(const BfContractData& bfItem)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (bfItem.symbol().length() == 0 || bfItem.exchange().length() == 0 || bfItem.name().length() == 0) {
        BfDebug("invalid contract,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;
    std::string key, val;

    if (1) {

        // key: contract-symbol-exchange
        key = QString().sprintf("contract-%s-%s", bfItem.symbol().c_str(), bfItem.exchange().c_str()).toStdString();
        bool ok = bfItem.SerializeToString(&val);
        if (!ok) {
            qFatal("SerializeToString fail");
        }
        batch.Put(key, val);

        // key: tick-symbol-exchange+
        // key: tick-symbol-exchange=
        BfTickData bfNullTick;
        key = QString().sprintf("tick-%s-%s+", bfItem.symbol().c_str(), bfItem.exchange().c_str()).toStdString();
        val = bfNullTick.SerializeAsString();
        batch.Put(key, val);
        key = QString().sprintf("tick-%s-%s=", bfItem.symbol().c_str(), bfItem.exchange().c_str()).toStdString();
        val = bfNullTick.SerializeAsString();
        batch.Put(key, val);

        // key: bar-symbol-exchange-period+
        // key: bar-symbol-exchange-period=
        BfBarData bfNullBar;
        for (int i = BfBarPeriod_MIN; i <= BfBarPeriod_MAX; i++) {
            key = QString().sprintf("bar-%s-%s-%s+", bfItem.symbol().c_str(), bfItem.exchange().c_str(), qPrintable(ProtoUtils::formatPeriod((BfBarPeriod)i))).toStdString();
            val = bfNullBar.SerializeAsString();
            batch.Put(key, val);
            key = QString().sprintf("bar-%s-%s-%s=", bfItem.symbol().c_str(), bfItem.exchange().c_str(), qPrintable(ProtoUtils::formatPeriod((BfBarPeriod)i))).toStdString();
            val = bfNullBar.SerializeAsString();
            batch.Put(key, val);
        }
    }

    db_->Write(options, &batch);
}

void DbService::deleteTick(const BfDeleteTickReq& bfReq)
{
    g_sm->checkCurrentOn(ServiceMgr::DB);
}

void DbService::deleteBar(const BfDeleteBarReq& bfReq)
{
    g_sm->checkCurrentOn(ServiceMgr::DB);
}

void DbService::deleteContract(const BfDatafeedDeleteContractReq& bfReq)
{
    g_sm->checkCurrentOn(ServiceMgr::DB);
}
