#include "pushservice.h"
#include "bfdatafeed.grpc.pb.h"
#include "bfgateway.pb.h"
#include "gatewaymgr.h"
#include "grpc++/grpc++.h"
#include "servicemgr.h"
#include <QDateTime>

using namespace bfdatafeed;
using namespace bfgateway;

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
        BfDebug(__FUNCTION__);
    }
    ~DatafeedClient()
    {
        BfDebug(__FUNCTION__);
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
};

//======
PushService::PushService(QObject* parent)
    : QObject(parent)
{
}

void PushService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // gatewaymgr
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotTick, this, &PushService::onGotTick);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotNotification, this, &PushService::onGotNotification);

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &PushService::onPing);
    this->pingTimer_->start();
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

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

void PushService::connectDatafeed(QString endpoint, QString clientId)
{
    BfInfo(__FUNCTION__);
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

void PushService::disconnectDatafeed()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (client_) {
        delete client_;
        client_ = nullptr;
    }
}

void PushService::onPing()
{
    //BfInfo(__FUNCTION__);
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

void PushService::onGotTick(QString gatewayId, const BfTickData& data)
{
    //BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (client_) {
        client_->InsertTick(data);
    }
}

void PushService::onGotNotification(QString gatewayId, const BfNotificationData& data)
{
    //BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (client_ && data.type() == NOTIFICATION_GOTCONTRACTS) {
        QList<BfContractData> resps;
        BfGetContractReq req;
        req.set_exchange("*");
        req.set_symbol("*");
        g_sm->gatewayMgr()->getContract(gatewayId, req, resps);

        for (auto resp : resps) {
            client_->InsertContract(resp);
        }
    }
}
