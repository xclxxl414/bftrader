#include "dbservice.h"
#include "bfdatafeed.grpc.pb.h"
#include "file_utils.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/write_batch.h"
#include "profile.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

//======
//
// DatafeedClient (sync)
//
class DatafeedClient {
public:
    DatafeedClient(std::shared_ptr<grpc::Channel> channel, QString clientId)
        : stub_(BfDatafeedService::NewStub(channel))
        , clientId_(clientId)
        , channel_(channel.get())
    {
        BfLog(__FUNCTION__);
    }
    ~DatafeedClient()
    {
        BfLog(__FUNCTION__);
    }

    bool ready()
    {
        if (GRPC_CHANNEL_READY == channel_->GetState(true)) {
            return true;
        }
        return false;
    }
    QString clientId() { return clientId_; }

public:
    // ref: grpc\test\cpp\interop\interop_client.cc
    bool Ping(const BfPingData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfPingData resp;
        grpc::Status status = stub_->Ping(&ctx, req, &resp);
        if (!status.ok()) {
            BfLog("Datafeed->Ping fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        if (req.message() != resp.message()) {
            BfLog("Datafeed->Ping fail,ping:%s,pong:%s", req.message().c_str(), resp.message().c_str());
            return false;
        }

        return true;
    }

    bool InsertTick(const BfTickData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid resp;
        grpc::Status status = stub_->InsertTick(&ctx, req, &resp);
        if (!status.ok()) {
            BfLog("Datafeed->InsertTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

    bool InsertBar(const BfBarData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid resp;
        grpc::Status status = stub_->InsertBar(&ctx, req, &resp);
        if (!status.ok()) {
            BfLog("Datafeed->InsertBar fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

    bool InsertContract(const BfContractData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid resp;
        grpc::Status status = stub_->InsertContract(&ctx, req, &resp);
        if (!status.ok()) {
            BfLog("Datafeed->InsertContract fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

    bool GetContract(const BfGetContractReq& req, QList<BfContractData>& resps)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5 * deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        int count = 0;
        BfLog("GetContract now");
        std::unique_ptr< ::grpc::ClientReader<BfContractData> > reader = stub_->GetContract(&ctx, req);
        for (;;) {
            BfContractData resp;
            bool ok = reader->Read(&resp);
            if (ok) {
                count++;
                resps.append(resp);
            } else {
                grpc::Status status = reader->Finish();
                if (!status.ok()) {
                    BfLog("Datafeed->GetContract fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                    return false;
                }
                break;
            }
        }
        BfLog("GetContract exit,count=(%d)", count);

        return true;
    }

    bool GetTick(const BfGetTickReq& req, QList<BfTickData>& resps)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5 * deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfLog("GetTick now");
        int count = 0;
        std::unique_ptr< ::grpc::ClientReader<BfTickData> > reader = stub_->GetTick(&ctx, req);
        for (;;) {
            BfTickData resp;
            bool ok = reader->Read(&resp);
            if (ok) {
                count++;
                resps.append(resp);
            } else {
                grpc::Status status = reader->Finish();
                if (!status.ok()) {
                    BfLog("Datafeed->GetTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                    return false;
                }
                break;
            }
        }
        BfLog("GetTick exit,count=(%d)", count);

        return true;
    }

    bool GetBar(const BfGetBarReq& req, QList<BfBarData>& resps)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5 * deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfLog("GetBar now");
        int count = 0;
        std::unique_ptr< ::grpc::ClientReader<BfBarData> > reader = stub_->GetBar(&ctx, req);
        for (;;) {
            BfBarData resp;
            bool ok = reader->Read(&resp);
            if (ok) {
                count++;
                resps.append(resp);
            } else {
                grpc::Status status = reader->Finish();
                if (!status.ok()) {
                    BfLog("Datafeed->GetBar fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                    return false;
                }
                break;
            }
        }
        BfLog("GetBar exit,count=(%d)", count);

        return true;
    }

    bool CleanAll()
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid req, resp;
        grpc::Status status = stub_->CleanAll(&ctx, req, &resp);
        if (!status.ok()) {
            BfLog("Datafeed->CleanAll fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

private:
    std::unique_ptr<BfDatafeedService::Stub> stub_;
    ::grpc::ChannelInterface* channel_;
    QString clientId_;
    const int deadline_ = 1000;
};

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

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &DbService::onPing);
    this->pingTimer_->start();
}

void DbService::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // close timer
    this->pingTimer_->stop();
    delete this->pingTimer_;
    this->pingTimer_ = nullptr;

    // free client
    if (client_) {
        delete client_;
        client_ = nullptr;
    }

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

void DbService::connectDatafeed(QString endpoint, QString clientId)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // datafeed client
    DatafeedClient* client = new DatafeedClient(
        grpc::CreateChannel(endpoint.toStdString(), grpc::InsecureChannelCredentials()),
        clientId);

    if (client_) {
        delete client_;
        client_ = nullptr;
    }

    client_ = client;
}

void DbService::disconnectDatafeed()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (client_) {
        delete client_;
        client_ = nullptr;
    }
}

void DbService::onPing()
{
    //BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (client_) {
        // ping datafeed
        BfPingData req;
        req.set_message(client_->clientId().toStdString());
        bool ok = client_->Ping(req);

        // ready是没必要的，用ping检测最好=
        if (ok || client_->ready()) {
        }
    }
}

bool DbService::getTick(const BfGetTickReq& req, QList<BfTickData>& resp)
{
    BfLog(__FUNCTION__);
    if (g_sm->isCurrentOn(ServiceMgr::MAIN)) {
        qFatal("cannt call in mainthread");
    }

    if (client_) {
        bool ok = client_->GetTick(req, resp);
        if (ok) {
            return true;
        }
    } else {
        BfLog("connectDatafeed firstly,plz");
    }

    return false;
}

bool DbService::getBar(const BfGetBarReq& req, QList<BfBarData>& resp)
{
    BfLog(__FUNCTION__);
    if (g_sm->isCurrentOn(ServiceMgr::MAIN)) {
        qFatal("cannt call in mainthread");
    }

    if (client_) {
        bool ok = client_->GetBar(req, resp);
        if (ok) {
            return true;
        }
    } else {
        BfLog("connectDatafeed firstly,plz");
    }

    return false;
}

bool DbService::getContract(const BfGetContractReq& req, QList<BfContractData>& resp)
{
    BfLog(__FUNCTION__);
    if (g_sm->isCurrentOn(ServiceMgr::MAIN)) {
        qFatal("cannt call in mainthread");
    }

    if (client_) {
        bool ok = client_->GetContract(req, resp);
        if (ok) {
            return true;
        }
    } else {
        BfLog("connectDatafeed firstly,plz");
    }

    return false;
}
