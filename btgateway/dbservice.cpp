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
        BfLog(__FUNCTION__);
    }
    ~DatafeedClient()
    {
        BfLog(__FUNCTION__);

        stopPushTick();
    }

    bool ready()
    {
        if (GRPC_CHANNEL_READY == channel_->GetState(true)) {
            return true;
        }
        return false;
    }
    QString clientId()
    {
        return clientId_;
    }
    bool isPushTickStopped()
    {
        return stopPushTick_;
    }
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

        BfLog("GetContract now");
        int count = 0;
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
        BfLog("GetContract exit,count=(%d)",count);

        return true;
    }

    void PushTick(const BfGetTickReq& req)
    {
        this->stopPushTick_ = false;
        reader_thread_ = new QThread();
        std::function<void(void)> fn = [=]() {
            grpc::ClientContext ctx;
            ctx.AddMetadata("clientid", clientId_.toStdString());
            std::unique_ptr< ::grpc::ClientReader<BfTickData> > reader = stub_->GetTick(&ctx, req);
            BfLog("PushTick begin...");
            int count = 0;
            for (;;) {
                BfTickData resp;
                bool ok = reader->Read(&resp);
                if (!this->isPushTickStopped() && ok) {
                    //BfLog("gotTick");
                    count++;
                    emit g_sm->dbService()->gotTick(resp);
                    Sleep(500);
                } else {
                    if (!ok) {
                        grpc::Status status = reader->Finish();
                        if (!status.ok()) {
                            BfLog("Datafeed->GetTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                        }
                    } else {
                        BfLog("!!!PushTick cancel, NOT call (reader->Finish), and (writer->Write) will hang!!!");
                    }
                    break;
                }
            }
            BfLog("PushTick end,total (%d) ticks...", count);
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
    BfLog(__FUNCTION__);
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
}

void DbService::connectDatafeed(QString endpoint, QString clientId)
{
    BfLog(__FUNCTION__);
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
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (client_) {
        delete client_;
        client_ = nullptr;
    }
}

void DbService::onPing()
{
    //BfLog(__FUNCTION__);
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
    BfLog(__FUNCTION__);
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
        BfLog("connectDatafeed firstly,plz");
    }
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

void DbService::onTradeStopped()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);

    if (client_) {
        client_->stopPushTick();
    }
}
