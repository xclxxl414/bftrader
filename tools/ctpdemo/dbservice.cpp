#include "dbservice.h"
#include "ThostFtdcUserApiStruct.h"
#include "ctp_utils.h"
#include "profile.h"
#include "utils.h"
#include "servicemgr.h"
#include "logger.h"
#include <QThread>
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/comparator.h"

//////
DbService::DbService(QObject* parent)
    : QObject(parent)
{
}

DbService::~DbService()
{
}

void DbService::init()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    leveldb::Env::Default();
    leveldb::BytewiseComparator();

    openInstrumentDB();
    openTickDB();
    openBarDB();

    initInstrumentLocator();
}

void DbService::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    closeInstrumentDB();
    closeTickDB();
    closeBarDB();

    delete leveldb::BytewiseComparator();
    delete leveldb::Env::Default();
}

void DbService::putTick(void* tick, int indexRb, void* rb)
{
    auto mdf = (CThostFtdcDepthMarketDataField*)tick;
    QString id = mdf->InstrumentID;
    auto db = getTickDB();
    QString key = QString().sprintf("%s-%s-%s-%d", mdf->InstrumentID, mdf->ActionDay, mdf->UpdateTime, mdf->UpdateMillisec);
    leveldb::Slice val((const char*)tick, sizeof(CThostFtdcDepthMarketDataField));
    leveldb::WriteOptions options;
    db->Put(options, key.toStdString(), val);
}

leveldb::DB* DbService::getInstrumentDB()
{
    if (instrument_db_ == nullptr) {
        qFatal("instrument_db_ == nullptr");
    }
    return instrument_db_;
}

leveldb::DB* DbService::getTickDB()
{
    if (tick_db_ == nullptr) {
        qFatal("tick_db_ == nullptr");
    }
    return tick_db_;
}

leveldb::DB* DbService::getBarDB()
{
    if (bar_db_ == nullptr) {
        qFatal("bar_db_ == nullptr");
    }
    return bar_db_;
}

void DbService::openBarDB(){
    QString path = Profile::barDbPath();
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

    bar_db_ = db;
}

void DbService::closeBarDB(){
    delete bar_db_;
}

void DbService::openTickDB(){
    QString path = Profile::tickDbPath();
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

    tick_db_ = db;
}

void DbService::closeTickDB(){
    delete tick_db_;
}

void DbService::openInstrumentDB(){
    QString path = Profile::instrumentDbPath();
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

    instrument_db_ = db;
}

void DbService::closeInstrumentDB(){
    delete instrument_db_;
}

//初始化instrument定位=
void DbService::initInstrumentLocator()
{
    CThostFtdcInstrumentField* idItem = new (CThostFtdcInstrumentField);
    memset(idItem, 0, sizeof(CThostFtdcInstrumentField));
    QString key;
    leveldb::Slice val((const char*)idItem, sizeof(CThostFtdcInstrumentField));
    leveldb::WriteOptions options;
    key = QStringLiteral("instrument+");
    instrument_db_->Put(options, key.toStdString(), val);
    key = QStringLiteral("instrument=");
    instrument_db_->Put(options, key.toStdString(), val);

    delete idItem;
}

//初始化tick定位=
void DbService::initTickLocator(QString id)
{
    leveldb::Slice val;
    void* buf;

    CThostFtdcDepthMarketDataField* tick = new (CThostFtdcDepthMarketDataField);
    memset(tick, 0, sizeof(CThostFtdcDepthMarketDataField));
    val = leveldb::Slice((const char*)tick, sizeof(CThostFtdcDepthMarketDataField));
    buf = tick;

    QString key;
    leveldb::WriteOptions options;
    key = id + QStringLiteral("+");
    tick_db_->Put(options, key.toStdString(), val);
    key = id + QStringLiteral("=");
    tick_db_->Put(options, key.toStdString(), val);

    delete buf;
}

//初始化bar定位=
void DbService::initBarLocator(QString id)
{
    BarItem* bar = new (BarItem);
    memset(bar, 0, sizeof(BarItem));
    QString key;
    leveldb::Slice val((const char*)bar, sizeof(BarItem));
    leveldb::WriteOptions options;
    key = id + QStringLiteral("+");
    bar_db_->Put(options, key.toStdString(), val);
    key = id + QStringLiteral("=");
    bar_db_->Put(options, key.toStdString(), val);

    delete bar;
}

void DbService::putInstrument(void* pInstrument)
{
    CThostFtdcInstrumentField* instrument = (CThostFtdcInstrumentField*)pInstrument;
    QString id = instrument->InstrumentID;

    QString key;
    leveldb::Slice val((const char*)instrument, sizeof(CThostFtdcInstrumentField));
    leveldb::WriteOptions options;
    key = QStringLiteral("instrument-") + id;
    instrument_db_->Put(options, key.toStdString(), val);

    initTickLocator(id);
    initBarLocator(id);
}
