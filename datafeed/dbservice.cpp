#include "dbservice.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "file_utils.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "logger.h"
#include "profile.h"
#include "servicemgr.h"

DbService::DbService(QObject* parent)
    : QObject(parent)
{
}

void DbService::init()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    leveldb::Env::Default();
    leveldb::BytewiseComparator();
}

void DbService::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    delete leveldb::BytewiseComparator();
    delete leveldb::Env::Default();
}

void DbService::dbOpen()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_) {
        g_sm->logger()->info("opened already");
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
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_ == nullptr) {
        g_sm->logger()->info("not open yet");
        return;
    }
    delete db_;
    db_ = nullptr;
}

void DbService::dbInit()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_ == nullptr) {
        g_sm->logger()->info("not open yet");
        return;
    }

    CThostFtdcInstrumentField* idItem = new (CThostFtdcInstrumentField);
    memset(idItem, 0, sizeof(CThostFtdcInstrumentField));
    QString key;
    leveldb::Slice val((const char*)idItem, sizeof(CThostFtdcInstrumentField));
    leveldb::WriteOptions options;
    key = QStringLiteral("instrument+");
    db_->Put(options, key.toStdString(), val);
    key = QStringLiteral("instrument=");
    db_->Put(options, key.toStdString(), val);
    delete idItem;
}
