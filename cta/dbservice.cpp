#include "dbservice.h"
#include "bfcta.pb.h"
#include "file_utils.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/write_batch.h"
#include "profile.h"
#include "servicemgr.h"

using namespace bfcta;

DbService::DbService(QObject* parent)
    : QObject(parent)
{
}

void DbService::init()
{
    BfLog(__FUNCTION__);
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
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // dbClose
    dbClose();

    // free env
    delete leveldb::BytewiseComparator();
    delete leveldb::Env::Default();
}

leveldb::DB* DbService::getDb()
{
    BfLog(__FUNCTION__);

    if (!db_) {
        qFatal("db not open yet");
        return nullptr;
    }

    return db_;
}

void DbService::dbOpen()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_) {
        BfLog("opened already");
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
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_ == nullptr) {
        BfLog("not open yet");
        return;
    }
    delete db_;
    db_ = nullptr;
}

void DbService::dbInit()
{
    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;

    if (1) {
        // key: order+
        // key: order=
        BfOrderData bfNullOrder;
        std::string key = "order+";
        std::string val = bfNullOrder.SerializeAsString();
        batch.Put(key, val);
        key = "order=";
        val = bfNullOrder.SerializeAsString();
        batch.Put(key, val);
    }

    if (1) {
        // key: trade+
        // key: trade=
        BfTradeData bfNullTrade;
        std::string key = "trade+";
        std::string val = bfNullTrade.SerializeAsString();
        batch.Put(key, val);
        key = "trade=";
        val = bfNullTrade.SerializeAsString();
        batch.Put(key, val);
    }

    db_->Write(options, &batch);
}
