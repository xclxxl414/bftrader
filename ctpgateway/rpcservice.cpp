#include "rpcservice.h"
#include "bfgateway.grpc.pb.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "pushservice.h"
#include "servicemgr.h"
#include <QThread>
#include <QtCore/QDebug>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfgateway;

//
// Gateway
// 全广播机制
//

class Gateway final : public BfGatewayService::Service {
public:
    Gateway()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Gateway()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ::grpc::Status Connect(::grpc::ServerContext* context, const ::bftrader::BfConnectReq* request, ::bftrader::BfConnectResp* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        BfDebug("peer:%s,%s:%s:%d", context->peer().c_str(), request->proxyid().c_str(), request->proxyip().c_str(), request->proxyport());
        QMetaObject::invokeMethod(g_sm->pushService(), "onProxyConnect", Qt::QueuedConnection, Q_ARG(BfConnectReq, *request));

        response->set_errorcode(0);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const ::bftrader::BfPingData* request, ::bftrader::BfPingData* response) override
    {
        QString proxyId = getProxyId(context);
        response->set_message(request->message());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status GetContract(::grpc::ServerContext* context, const ::bftrader::BfGetContractReq* request, ::bftrader::BfContractData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString proxyId = getProxyId(context);
        BfDebug("proxyid=%s", qPrintable(proxyId));

        int index = request->index();
        if (index <= 0) {
            QString symbol = request->symbol().c_str();
            void* contract = g_sm->ctpMgr()->getContract(symbol);
            CtpUtils::translateContract(contract, response);
        } else {
            QStringList ids = request->subscribled() ? g_sm->ctpMgr()->getIds() : g_sm->ctpMgr()->getIdsAll();
            if (ids.length() > index - 1) {
                QString symbol = ids.at(index - 1);
                void* contract = g_sm->ctpMgr()->getContract(symbol);
                CtpUtils::translateContract(contract, response);
            }
        }

        return grpc::Status::OK;
    }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const ::bftrader::BfSendOrderReq* request, ::bftrader::BfSendOrderResp* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString proxyId = getProxyId(context);
        BfDebug("proxyid=%s", qPrintable(proxyId));

        QString bfOrderId = g_sm->ctpMgr()->genOrderId();
        response->set_bforderid(bfOrderId.toStdString());

        QMetaObject::invokeMethod(g_sm->ctpMgr(), "sendOrderWithId", Qt::QueuedConnection, Q_ARG(QString, bfOrderId), Q_ARG(BfSendOrderReq, *request));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const ::bftrader::BfCancelOrderReq* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString proxyId = getProxyId(context);
        BfDebug("proxyid=%s", qPrintable(proxyId));

        QMetaObject::invokeMethod(g_sm->ctpMgr(), "cancelOrder", Qt::QueuedConnection, Q_ARG(BfCancelOrderReq, *request));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status QueryAccount(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString proxyId = getProxyId(context);
        BfDebug("proxyid=%s", qPrintable(proxyId));

        QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryAccount", Qt::QueuedConnection);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status QueryPosition(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString proxyId = getProxyId(context);
        BfDebug("proxyid=%s", qPrintable(proxyId));

        QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryPosition", Qt::QueuedConnection);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Close(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString proxyId = getProxyId(context);
        BfDebug("proxyid=%s", qPrintable(proxyId));

        QMetaObject::invokeMethod(g_sm->pushService(), "onProxyClose", Qt::QueuedConnection, Q_ARG(QString, proxyId));
        return grpc::Status::OK;
    }

private:
    // metadata-key只能是小写的=
    QString getProxyId(::grpc::ServerContext* context)
    {
        QString proxyId;
        if (0 != context->client_metadata().count("proxyid")) {
            auto its = context->client_metadata().equal_range("proxyid");
            auto it = its.first;
            proxyId = grpc::string(it->second.begin(), it->second.end()).c_str();
            BfDebug("metadata: proxyid=%s", proxyId.toStdString().c_str());
        }
        return proxyId;
    }
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

    stop();
}

void RpcService::start()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (gatewayThread_ == nullptr) {
        gatewayThread_ = new QThread();
        QObject::connect(gatewayThread_, &QThread::started, this, &RpcService::onGatewayThreadStarted, Qt::DirectConnection);
        gatewayThread_->start();
    }
}

void RpcService::stop()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (gatewayThread_ != nullptr) {
        grpcServer_->Shutdown();
        grpcServer_ = nullptr;

        gatewayThread_->quit();
        gatewayThread_->wait();
        delete gatewayThread_;
        gatewayThread_ = nullptr;

        QMetaObject::invokeMethod(g_sm->pushService(), "onGatewayClose", Qt::QueuedConnection);
    }
}

void RpcService::onGatewayThreadStarted()
{
    BfDebug(__FUNCTION__);
    if (g_sm->isCurrentOn(ServiceMgr::RPC)) {
        qFatal("g_sm->CurrentOn(ServiceMgr::RPC)");
    }

    std::string server_address("0.0.0.0:50051");
    Gateway gateway;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&gateway);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfInfo(QString("gateway listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfInfo(QString("gateway shutdown"));
}
