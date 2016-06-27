#include "dbservice.h"
#include "bfdatafeed.grpc.pb.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "gatewaymgr.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/write_batch.h"
#include "profile.h"
#include "protoutils.h"
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

    emit this->opened();
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
    //BfDebug(__FUNCTION__);

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
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);
    if (db_ == nullptr) {
        qFatal("db not open yet");
        return;
    }

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

void DbService::dbReset()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);
    if (db_ != nullptr) {
        qFatal("db open yet");
        return;
    }

    QString path = Profile::dbPath();
    mkDir(path);
    leveldb::Options options;
    leveldb::Status status = leveldb::DestroyDB(path.toStdString(), options);
    if (!status.ok()) {
        qFatal("leveldb::DestroyDB fail");
    }
}

void DbService::insertTick(const BfTickData& bfItem)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (bfItem.symbol().length() == 0 || bfItem.exchange().length() == 0 || bfItem.actiondate().length() == 0 || bfItem.ticktime().length() == 0) {
        BfDebug("invalid tick,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;
    std::string key, val;

    if (1) {
        // key: tick-symbol-exchange-actiondate-ticktime
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
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (bfItem.symbol().length() == 0 || bfItem.exchange().length() == 0 || bfItem.actiondate().length() == 0 || bfItem.bartime().length() == 0 || bfItem.period() == PERIOD_UNKNOWN) {
        BfDebug("invalid bar,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;
    std::string key, val;

    if (1) {
        // key: bar-symbol-exchange-period-actiondate-bartime
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
    //BfDebug(__FUNCTION__);
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

void DbService::getTick(const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    if (request->symbol().length() == 0 || request->exchange().length() == 0 || request->todate().length() == 0 || request->totime().length() == 0) {
        BfDebug("invalid parm,ignore");
        return;
    }

    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db_->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    // key: tick-symbol-exchange-actiondate-ticktime
    std::string key = QString().sprintf("tick-%s-%s-%s-%s", request->symbol().c_str(), request->exchange().c_str(), request->todate().c_str(), request->totime().c_str()).toStdString();
    std::string lastestKey = QString().sprintf("tick-%s-%s=", request->symbol().c_str(), request->exchange().c_str()).toStdString();
    std::string itKey;
    it->Seek(leveldb::Slice(key));
    int count = 0;
    for (; it->Valid() && count < request->count(); it->Prev()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfTickData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            itKey = it->key().ToString();
            if (itKey == lastestKey) {
                continue;
            } else {
                break;
            }
        }

        count++;
        writer->Write(bfItem);
    }
    delete it;
}

void DbService::getBar(const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    if (request->symbol().length() == 0 || request->exchange().length() == 0
        || request->todate().length() == 0 || request->totime().length() == 0
        || request->period() == PERIOD_UNKNOWN) {
        BfDebug("invalid parm,ignore");
        return;
    }

    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db_->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    // key: bar-symbol-exchange-period-actiondate-bartime
    std::string key = QString().sprintf("bar-%s-%s-%s-%s-%s", request->symbol().c_str(), request->exchange().c_str(),
                                   qPrintable(ProtoUtils::formatPeriod(request->period())),
                                   request->todate().c_str(), request->totime().c_str())
                          .toStdString();
    std::string lastestKey = QString().sprintf("bar-%s-%s-%s=", request->symbol().c_str(), request->exchange().c_str(),
                                          qPrintable(ProtoUtils::formatPeriod(request->period())))
                                 .toStdString();
    std::string itKey;
    it->Seek(leveldb::Slice(key));
    int count = 0;
    for (; it->Valid() && count < request->count(); it->Prev()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfBarData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            itKey = it->key().ToString();
            if (itKey == lastestKey) {
                continue;
            } else {
                break;
            }
        }

        count++;
        writer->Write(bfItem);
    }
    delete it;
}

void DbService::getContract(const BfGetContractReq* request, ::grpc::ServerWriter<BfContractData>* writer)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    if (request->symbol().length() == 0 || request->exchange().length() == 0) {
        BfDebug("invalid parm,ignore");
        return;
    }

    if (request->symbol() == "*" && request->exchange() == "*") {
        leveldb::DB* db = db_;
        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        //第一个是contract+
        //最后一个是contract=
        QString key = QStringLiteral("contract+");
        it->Seek(leveldb::Slice(key.toStdString()));
        if (it->Valid()) {
            it->Next();
        }
        for (; it->Valid(); it->Next()) {
            //遇到了前后两个结束item
            const char* buf = it->value().data();
            int len = it->value().size();
            BfContractData bfContract;
            //std::string stdKey = it->key().ToString();
            //std::string stdVal = it->value().ToString();
            //if(!bfContract.ParseFromString(stdVal)){
            if (!bfContract.ParseFromArray(buf, len)) {
                qFatal("ParseFromArray fail");
                break;
            }
            if (bfContract.symbol().length() == 0) {
                break;
            }

            writer->Write(bfContract);
        }
        delete it;

    } else {
        leveldb::ReadOptions options;
        std::string val;
        // key: contract-symbol-exchange
        std::string key = QString().sprintf("contract-%s-%s", request->symbol().c_str(), request->exchange().c_str()).toStdString();
        leveldb::Status status = db_->Get(options, key, &val);
        if (status.ok()) {
            BfContractData bfItem;
            if (!bfItem.ParseFromString(val)) {
                qFatal("ParseFromString error");
                return;
            }
            if (bfItem.symbol().length() == 0) {
                return;
            }
            writer->Write(bfItem);
        }
    }
}

// 遍历构建batch，然后执行batch
void DbService::deleteTick(const BfDeleteTickReq& request)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (request.symbol().length() == 0 || request.exchange().length() == 0
        || request.todate().length() == 0 || request.totime().length() == 0
        || request.fromdate().length() == 0 || request.fromtime().length() == 0) {
        BfDebug("invalid parm,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;

    // key: tick-symbol-exchange-actiondate-ticktime
    std::string fromKey, endKey, firstKey, lastestKey, itKey;
    fromKey = QString().sprintf("tick-%s-%s-%s-%s", request.symbol().c_str(), request.exchange().c_str(), request.fromdate().c_str(), request.fromtime().c_str()).toStdString();
    endKey = QString().sprintf("tick-%s-%s-%s-%s", request.symbol().c_str(), request.exchange().c_str(), request.todate().c_str(), request.totime().c_str()).toStdString();
    firstKey = QString().sprintf("tick-%s-%s+", request.symbol().c_str(), request.exchange().c_str()).toStdString();
    lastestKey = QString().sprintf("tick-%s-%s=", request.symbol().c_str(), request.exchange().c_str()).toStdString();

    if (1) {
        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db_->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        it->Seek(leveldb::Slice(fromKey));
        for (; it->Valid(); it->Next()) {
            itKey = it->key().ToString();
            if (itKey == firstKey) {
                continue;
            }
            if (itKey == lastestKey) {
                break;
            }
            if (itKey <= endKey) {
                batch.Delete(itKey);
            } else {
                break;
            }
        }
        delete it;
    }

    db_->Write(options, &batch);
}

// 遍历构建batch，然后执行batch
void DbService::deleteBar(const BfDeleteBarReq& request)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);
    if (request.symbol().length() == 0 || request.exchange().length() == 0
        || request.todate().length() == 0 || request.totime().length() == 0
        || request.fromdate().length() == 0 || request.fromtime().length() == 0
        || request.period() == PERIOD_UNKNOWN) {
        BfDebug("invalid parm,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;

    // key: bar-symbol-exchange-period-actiondate-bartime
    std::string fromKey, endKey, firstKey, lastestKey, itKey;
    fromKey = QString().sprintf("tick-%s-%s-%s-%s-%s", request.symbol().c_str(), request.exchange().c_str(),
                           qPrintable(ProtoUtils::formatPeriod(request.period())),
                           request.fromdate().c_str(), request.fromtime().c_str())
                  .toStdString();
    endKey = QString().sprintf("tick-%s-%s-%s-%s-%s", request.symbol().c_str(), request.exchange().c_str(),
                          qPrintable(ProtoUtils::formatPeriod(request.period())),
                          request.todate().c_str(), request.totime().c_str())
                 .toStdString();
    firstKey = QString().sprintf("tick-%s-%s-%s+", request.symbol().c_str(), request.exchange().c_str(),
                            qPrintable(ProtoUtils::formatPeriod(request.period())))
                   .toStdString();
    lastestKey = QString().sprintf("tick-%s-%s-%s=", request.symbol().c_str(), request.exchange().c_str(),
                              qPrintable(ProtoUtils::formatPeriod(request.period())))
                     .toStdString();

    if (1) {
        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db_->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        it->Seek(leveldb::Slice(fromKey));
        for (; it->Valid(); it->Next()) {
            itKey = it->key().ToString();
            if (itKey == firstKey) {
                continue;
            }
            if (itKey == lastestKey) {
                break;
            }
            if (itKey <= endKey) {
                batch.Delete(itKey);
            } else {
                break;
            }
        }
        delete it;
    }

    db_->Write(options, &batch);
}

void DbService::deleteContract(const BfDeleteContractReq& request)
{
    //BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);
    if (request.symbol().length() == 0 || request.exchange().length() == 0) {
        BfDebug("invalid parm,ignore");
        return;
    }

    leveldb::WriteOptions options;
    leveldb::WriteBatch batch;

    if (request.symbol() == "*" && request.exchange() == "*") {
        leveldb::DB* db = db_;
        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        //第一个是contract+
        //最后一个是contract=
        std::string firstKey = "contract+";
        std::string lastestKey = "contract=";
        std::string itKey;
        it->Seek(firstKey);
        if (it->Valid()) {
            it->Next();
        }
        for (; it->Valid(); it->Next()) {
            itKey = it->key().ToString();
            if (itKey == firstKey) {
                continue;
            }
            if (itKey == lastestKey) {
                break;
            }
            batch.Delete(itKey);
        }
        delete it;
    } else {
        std::string key = QString().sprintf("contract-%s-%s", request.symbol().c_str(), request.exchange().c_str()).toStdString();
        batch.Delete(key);
    }

    db_->Write(options, &batch);
}

void DbService::cleanAll()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // dbClose
    dbClose();

    // dbReset
    dbReset();

    // dbOpen
    dbOpen();

    // dbInit
    dbInit();

    emit this->opened();
}

void DbService::dbCompact()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    db_->CompactRange(nullptr,nullptr);
}
