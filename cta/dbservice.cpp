#include "dbservice.h"
#include "file_utils.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
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

    leveldb::Env::Default();
    leveldb::BytewiseComparator();
}

void DbService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    delete leveldb::BytewiseComparator();
    delete leveldb::Env::Default();
}

void DbService::dbOpen()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (db_) {
        BfDebug("opened already");
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
        BfDebug("not open yet");
        return;
    }
    delete db_;
    db_ = nullptr;
}

// TODO(hege):do it
QString DbService::getRobotId(const BfOrderData& bfItem)
{
    return "demo";
}

// TODO(hege):do it
QString DbService::getRobotId(const BfTradeData& bfItem)
{
    return "demo";
}

// TODO(hege):do it
QString DbService::getGatewayId(const BfConnectReq& bfItem)
{
    return "ctpGateway";
}

// TODO(hege):do it
QString DbService::getGatewayId(const QString& robotId)
{
    return "ctpGateway";
}
