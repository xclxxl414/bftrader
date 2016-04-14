#include "leveldbbackend.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/comparator.h"
#include "servicemgr.h"
#include "logger.h"
#include "profile.h"
#include "utils.h"
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcMdApi.h"

LeveldbBackend::LeveldbBackend(QObject *parent) : QObject(parent)
{

}

void LeveldbBackend::init(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    leveldb::Env::Default();
    leveldb::BytewiseComparator();
}

void LeveldbBackend::shutdown(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    delete leveldb::BytewiseComparator();
    delete leveldb::Env::Default();
}

void LeveldbBackend::dbOpen(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_){
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

void LeveldbBackend::dbClose(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if(db_ == nullptr){
        g_sm->logger()->info("not open yet");
        return;
    }
    delete db_;
    db_ = nullptr;
}

void LeveldbBackend::dbInit(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if(db_ == nullptr){
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
