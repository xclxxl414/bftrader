#include "rpcservice.h"
#include "bfcta.grpc.pb.h"
#include "dbservice.h"
#include "pushservice.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfcta;

//
// cta的rpc可以直接调用gatewaymgr的slots以调用gateway，grpc是多线程安全的=
//
// todo(hege):在connect时候，需要核对modelId RobotId信息哦=
//
class Cta final : public BfCtaService::Service {
public:
    explicit Cta(QString ctaId)
        : ctaId_(ctaId)
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Cta()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ::grpc::Status Connect(::grpc::ServerContext* context, const ::bftrader::BfConnectReq* request, ::bftrader::BfConnectResp* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        BfDebug("peer:%s,%s:%s:%d", context->peer().c_str(), request->clientid().c_str(), request->clientip().c_str(), request->clientport());
        QMetaObject::invokeMethod(g_sm->pushService(), "connectRobot", Qt::QueuedConnection, Q_ARG(QString, ctaId_), Q_ARG(BfConnectReq, *request));

        response->set_errorcode(0);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const ::bftrader::BfPingData* request, ::bftrader::BfPingData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        response->set_message(request->message());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Disconnect(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfDebug("clientId=%s", qPrintable(clientId));

        QMetaObject::invokeMethod(g_sm->pushService(), "disconnectRobot", Qt::QueuedConnection, Q_ARG(QString, clientId));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status GetRobotInfo(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfKvData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const ::bftrader::BfSendOrderReq* request, ::bftrader::BfSendOrderResp* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfDebug("clientId=%s", qPrintable(clientId));

        QString gatewayId = g_sm->dbService()->getGatewayId(clientId);
        g_sm->gatewayMgr()->sendOrder(gatewayId, *request, *response);

        return grpc::Status::OK;
    }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const ::bftrader::BfCancelOrderReq* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfDebug("clientId=%s", qPrintable(clientId));

        QString gatewayId = g_sm->dbService()->getGatewayId(clientId);
        g_sm->gatewayMgr()->cancelOrder(gatewayId, *request);

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
            //BfDebug("metadata: clientid=%s", clientId.toStdString().c_str());
        }
        return clientId;
    }

private:
    QString ctaId_;
};

//
// RpcService
//
RpcService::RpcService(QObject* parent)
    : QObject(parent)
{
}

void RpcService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void RpcService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void RpcService::start()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (ctaThread_ == nullptr) {
        ctaThread_ = new QThread();
        QObject::connect(ctaThread_, &QThread::started, this, &RpcService::onCtaThreadStarted, Qt::DirectConnection);
        ctaThread_->start();
    }
}

void RpcService::stop()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (ctaThread_ != nullptr) {
        grpcServer_->Shutdown();
        grpcServer_ = nullptr;

        ctaThread_->quit();
        ctaThread_->wait();
        delete ctaThread_;
        ctaThread_ = nullptr;

        QMetaObject::invokeMethod(g_sm->pushService(), "onCtaClosed", Qt::QueuedConnection);
    }
}

void RpcService::onCtaThreadStarted()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    std::string server_address("0.0.0.0:50054");
    Cta cta("cta");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&cta);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfInfo(QString("cta listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfInfo(QString("cta shutdown"));
}
