#include "gatewaymgr.h"
#include "bfgateway.grpc.pb.h"
#include "bfproxy.grpc.pb.h"
#include "servicemgr.h"
#include <QThread>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfproxy;
using namespace bftrader::bfgateway;

//
// Proxy
//

class Proxy final : public BfProxyService::Service {
public:
    Proxy()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Proxy()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ::grpc::Status OnTradeWillBegin(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnGotContracts(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnPing(::grpc::ServerContext* context, const ::bftrader::BfPingData* request, ::bftrader::BfPingData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnTick(::grpc::ServerContext* context, const ::bftrader::BfTickData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnError(::grpc::ServerContext* context, const ::bftrader::BfErrorData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnLog(::grpc::ServerContext* context, const ::bftrader::BfLogData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnTrade(::grpc::ServerContext* context, const ::bftrader::BfTradeData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnOrder(::grpc::ServerContext* context, const ::bftrader::BfOrderData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnPosition(::grpc::ServerContext* context, const ::bftrader::BfPositionData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnAccount(::grpc::ServerContext* context, const ::bftrader::BfAccountData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }

private:
    // metadata-key只能是小写的=
    QString getClientId(::grpc::ServerContext* context)
    {
        QString clientId;
        if (0 != context->client_metadata().count("clientid")) {
            auto its = context->client_metadata().equal_range("clientid");
            auto it = its.first;
            clientId = grpc::string(it->second.begin(), it->second.end()).c_str();
            BfDebug("metadata: clientid=%s", clientId.toStdString().c_str());
        }
        return clientId;
    }
};

//
// GatewayClient
//
class GatewayClient {
public:
    GatewayClient(std::shared_ptr<grpc::Channel> channel, QString gatewayId, const BfConnectReq& req)
        : stub_(BfGatewayService::NewStub(channel))
        , gatewayId_(gatewayId)
        , req_(req)
    {
        BfDebug(__FUNCTION__);
    }
    ~GatewayClient() {}

    // ref: grpc\test\cpp\interop\interop_client.cc
    void Ping(const BfPingData& req, BfPingData& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->Ping(&ctx, req, &resp);
        if (!status.ok()) {
            pingfail_count_++;
            BfError("(%s)->Ping fail(%d),code:%d,msg:%s", qPrintable(gatewayId_), pingfail_count_, status.error_code(), status.error_message().c_str());
            //if (pingfail_count_ > 3) {
            //    BfError("(%s)->Ping fail too long,so kill it", qPrintable(clientId_));
            //    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "disconnectGateway", Qt::QueuedConnection, Q_ARG(QString, clientId_));
            //}
            return;
        }
        pingfail_count_ = 0;

        if (req.message() != resp.message()) {
            BfError("(%s)->Ping fail,ping:%s,pong:%s", qPrintable(gatewayId_), req.message().c_str(), resp.message().c_str());
            return;
        }
    }

    void Connect(const BfConnectReq& req, BfConnectResp& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->Connect(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->Connect,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

    void Disconnect(const BfVoid& req, BfVoid& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->Disconnect(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->Disconnect,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

public:
    bool logHandler() { return req_.loghandler(); }
    bool tickHandler() { return req_.tickhandler(); }
    bool tradehandler() { return req_.tradehandler(); }
    bool subscribled(const std::string& symbol, const std::string& exchange)
    {
        if (req_.symbol() == "*") {
            return true;
        }
        if (symbol == req_.symbol()) {
            return true;
        }
        return false;
    }

private:
    std::unique_ptr<BfGatewayService::Stub> stub_;
    int pingfail_count_ = 0;
    const int deadline_ = 500;
    QString gatewayId_;
    BfConnectReq req_;
};

//
// GatewayMgr
//
GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // qRegisterMetaType
    qRegisterMetaType<BfAccountData>("BfAccountData");
    qRegisterMetaType<BfPositionData>("BfPositionData");
    qRegisterMetaType<BfOrderData>("BfOrderData");
    qRegisterMetaType<BfTradeData>("BfTradeData");
    qRegisterMetaType<BfSendOrderReq>("BfSendOrderReq");
    qRegisterMetaType<BfCancelOrderReq>("BfCancelOrderReq");
    qRegisterMetaType<BfConnectReq>("BfConnectReq");

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &GatewayMgr::onPing);
    this->pingTimer_->start();
}

void GatewayMgr::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // close timer
    this->pingTimer_->stop();
    delete this->pingTimer_;
    this->pingTimer_ = nullptr;

    // delete all gatewayclient
    for (auto client : clients_) {
        delete client;
    }
    clients_.clear();

    // stop proxy
    stopProxy();
}

void GatewayMgr::startProxy()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (proxyThread_ == nullptr) {
        proxyThread_ = new QThread();
        QObject::connect(proxyThread_, &QThread::started, this, &GatewayMgr::onProxyThreadStarted, Qt::DirectConnection);
        proxyThread_->start();
    }
}

void GatewayMgr::stopProxy()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (proxyThread_ != nullptr) {
        grpcServer_->Shutdown();
        grpcServer_ = nullptr;

        proxyThread_->quit();
        proxyThread_->wait();
        delete proxyThread_;
        proxyThread_ = nullptr;

        QMetaObject::invokeMethod(g_sm->gatewayMgr(), "onProxyClosed", Qt::QueuedConnection);
    }
}

void GatewayMgr::onProxyThreadStarted()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    std::string server_address("0.0.0.0:50053");
    Proxy proxy;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&proxy);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfInfo(QString("proxy listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfInfo(QString("proxy shutdown"));
}

void GatewayMgr::connectGateway(QString gatewayId, QString endpoint, const BfConnectReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // gatewayclient
    GatewayClient* client = new GatewayClient(grpc::CreateChannel(endpoint.toStdString(), grpc::InsecureChannelCredentials()),
        gatewayId, req);

    // cache
    if (clients_.contains(gatewayId)) {
        auto it = clients_[gatewayId];
        delete it;
        clients_.remove(gatewayId);
    }
    clients_[gatewayId] = client;

    // connecct
    BfConnectResp resp;
    client->Connect(req, resp);
}

void GatewayMgr::disconnectGateway(QString gatewayId)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (clients_.contains(gatewayId)) {
        BfDebug("delete gatewayclient:%s", qPrintable(gatewayId));
        GatewayClient* client = clients_[gatewayId];

        // disconnect
        BfVoid req, resp;
        client->Disconnect(req, resp);

        // free
        delete client;
        clients_.remove(gatewayId);
    }
}

void GatewayMgr::onProxyClosed()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    for (auto client : clients_) {
        // disconnect
        BfVoid req, resp;
        client->Disconnect(req, resp);

        // free
        delete client;
    }
    clients_.clear();
}

void GatewayMgr::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfPingData req, resp;
    req.set_message("cta");
    for (auto client : clients_) {
        client->Ping(req, resp);
    }
}

void GatewayMgr::getContract(const BfGetContractReq& req, BfContractData& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
void GatewayMgr::sendOrder(const BfSendOrderReq& req, BfSendOrderResp& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
void GatewayMgr::cancelOrder(const BfCancelOrderReq& req, BfVoid& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
void GatewayMgr::queryAccount(const BfVoid& req, BfVoid& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
void GatewayMgr::queryPosition(const BfVoid& req, BfVoid& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
void GatewayMgr::queryOrders(const BfVoid& req, BfVoid& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
