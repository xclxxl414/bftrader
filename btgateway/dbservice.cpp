#include "dbservice.h"
#include "bfdatafeed.grpc.pb.h"
#include "file_utils.h"
#include "profile.h"
#include "servicemgr.h"
#include <QThread>
#include <grpc++/grpc++.h>

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
        BfDebug(__FUNCTION__);
    }
    ~DatafeedClient()
    {
        BfDebug(__FUNCTION__);

        stopPushTick();
    }

    bool ready()
    {
        if (GRPC_CHANNEL_READY == channel_->GetState(true)) {
            return true;
        }
        return false;
    }
    QString clientId() { return clientId_; }

    void stopPushTick()
    {
        stopPushTick_ = true;

        if (reader_thread_) {
            reader_thread_->quit();
            reader_thread_->wait();
            delete reader_thread_;
            reader_thread_ = nullptr;
        }
    }

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
            BfError("Datafeed->Ping fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        if (req.message() != resp.message()) {
            BfError("Datafeed->Ping fail,ping:%s,pong:%s", req.message().c_str(), resp.message().c_str());
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
            BfError("Datafeed->InsertTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
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
            BfError("Datafeed->InsertBar fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
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
            BfError("Datafeed->InsertContract fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
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

        std::unique_ptr< ::grpc::ClientReader<BfContractData> > reader = stub_->GetContract(&ctx, req);
        for (;;) {
            BfContractData resp;
            bool ok = reader->Read(&resp);
            if (ok) {
                resps.append(resp);
            } else {
                grpc::Status status = reader->Finish();
                if (!status.ok()) {
                    BfError("Datafeed->GetContract fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                    return false;
                }
                break;
            }
        }

        return true;
    }

    void PushTick(const BfGetTickReq& req)
    {
        stopPushTick_ = false;
        reader_thread_ = new QThread();
        std::function<void(void)> fn = [=]() {
            grpc::ClientContext ctx;
            ctx.AddMetadata("clientid", clientId_.toStdString());
            std::unique_ptr< ::grpc::ClientReader<BfTickData> > reader = stub_->GetTick(&ctx, req);
            for (;;) {
                BfTickData resp;
                bool ok = reader->Read(&resp);
                if (!stopPushTick_ && ok) {
                    emit g_sm->dbService()->gotTick(resp);
                    Sleep(500);
                } else {
                    grpc::Status status = reader->Finish();
                    if (!status.ok()) {
                        BfError("Datafeed->GetTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                    }
                    break;
                }
            }
        };
        QObject::connect(reader_thread_, &QThread::started, fn);
        reader_thread_->start();
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
            BfError("Datafeed->CleanAll fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

private:
    std::unique_ptr<BfDatafeedService::Stub> stub_;
    ::grpc::ChannelInterface* channel_;
    QString clientId_;
    const int deadline_ = 1000;
    QThread* reader_thread_ = nullptr;
    bool stopPushTick_ = false;
};

//======
DbService::DbService(QObject* parent)
    : QObject(parent)
{
}

void DbService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // gatewaymgr...
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::tradeWillBegin, this, &DbService::onTradeWillBegin);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::tradeStopped, this, &DbService::onTradeStopped);

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &DbService::onPing);
    this->pingTimer_->start();
}

void DbService::shutdown()
{
    BfDebug(__FUNCTION__);
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
}

void DbService::connectDatafeed(QString endpoint, QString clientId)
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

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
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (client_) {
        delete client_;
        client_ = nullptr;
    }
}

void DbService::onPing()
{
    //BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

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

void DbService::onTradeWillBegin(const BfGetTickReq& reqTick)
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    // wait 1 second,同ctpgateway设计，让策略有时间做一些破事情=
    Sleep(1000);

    if (client_) {
        QList<BfContractData> resps;
        BfGetContractReq req;
        req.set_exchange("*");
        req.set_symbol("*");
        bool ok = client_->GetContract(req, resps);
        if (ok) {
            emit this->gotContracts();

            // wait 1 second
            Sleep(1000);
            client_->PushTick(reqTick);
        }
    } else {
        BfError("connectDatafeed firstly,plz");
    }
}

void DbService::getContract(const BfGetContractReq& req, QList<BfContractData>& resp)
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    if (client_) {
        bool ok = client_->GetContract(req, resp);
        if (ok) {
            ;
        }
    } else {
        BfError("connectDatafeed firstly,plz");
    }
}

void DbService::onTradeStopped()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (client_) {
        client_->stopPushTick();
    }
}
