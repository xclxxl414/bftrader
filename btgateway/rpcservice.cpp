#include "rpcservice.h"
#include "bfgateway.grpc.pb.h"
#include "dbservice.h"
#include "encode_utils.h"
#include "gatewaymgr.h"
#include "pushservice.h"
#include "safequeue.h"
#include "servicemgr.h"
#include <QThread>
#include <QtCore/QDebug>
#include <grpc++/grpc++.h>

//
// Gateway,动态多线程服务器=
//

class Gateway final : public BfGatewayService::Service {
public:
    explicit Gateway(QString gatewayId)
        : gatewayId_(gatewayId)
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Gateway()
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const BfPingData* request, BfPingData* response) override
    {
        QString clientId = getClientId(context);
        response->set_message(request->message());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status ConnectPush(::grpc::ServerContext* context, const BfConnectPushReq* request, ::grpc::ServerWriter< ::google::protobuf::Any>* writer) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        QString clientId = request->clientid().c_str();
        BfLog("(%s)->Connect", qPrintable(clientId));

        auto queue = new SafeQueue<google::protobuf::Any>;
        QMetaObject::invokeMethod(g_sm->pushService(), "connectClient", Qt::QueuedConnection, Q_ARG(QString, gatewayId_), Q_ARG(BfConnectPushReq, *request), Q_ARG(void*, (void*)queue));
        while (auto data = queue->dequeue()) {
            // NOTE(hege):客户端异常导致stream关闭
            bool ok = writer->Write(*data);
            delete data;
            if (!ok) {
                BfLog("(%s)-->stream closed!", qPrintable(clientId));
                QMetaObject::invokeMethod(g_sm->pushService(), "disconnectClient", Qt::QueuedConnection, Q_ARG(QString, clientId));
                break;
            }
        }

        BfLog("(%s)->Connect exit!", qPrintable(clientId));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status DisconnectPush(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("(%s)->Disconnect", qPrintable(clientId));

        // NOTE(hege):关闭stream
        QMetaObject::invokeMethod(g_sm->pushService(), "disconnectClient", Qt::QueuedConnection, Q_ARG(QString, clientId));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status GetContract(::grpc::ServerContext* context, const BfGetContractReq* request, ::grpc::ServerWriter<BfContractData>* writer) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("clientId=%s", qPrintable(clientId));

        QList<BfContractData> resps;
        g_sm->dbService()->getContract(*request, resps);
        for (auto resp : resps) {
            writer->Write(resp);
        }

        return grpc::Status::OK;
    }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const BfSendOrderReq* request, BfSendOrderResp* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("clientId=%s", qPrintable(clientId));

        QString bfOrderId = g_sm->gatewayMgr()->genOrderId();
        response->set_bforderid(bfOrderId.toStdString());

        QMetaObject::invokeMethod(g_sm->gatewayMgr(), "sendOrderWithId", Qt::QueuedConnection, Q_ARG(QString, bfOrderId), Q_ARG(BfSendOrderReq, *request));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const BfCancelOrderReq* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("clientId=%s", qPrintable(clientId));

        QMetaObject::invokeMethod(g_sm->gatewayMgr(), "cancelOrder", Qt::QueuedConnection, Q_ARG(BfCancelOrderReq, *request));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status QueryAccount(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("clientId=%s", qPrintable(clientId));

        QMetaObject::invokeMethod(g_sm->gatewayMgr(), "queryAccount", Qt::QueuedConnection);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status QueryPosition(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("clientId=%s", qPrintable(clientId));

        QMetaObject::invokeMethod(g_sm->gatewayMgr(), "queryPosition", Qt::QueuedConnection);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status QueryOrders(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfLog("clientId=%s", qPrintable(clientId));

        QMetaObject::invokeMethod(g_sm->gatewayMgr(), "queryOrders", Qt::QueuedConnection);
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
            //BfLog("metadata: clientid=%s", clientId.toStdString().c_str());
        }
        return clientId;
    }

private:
    QString gatewayId_;
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
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void RpcService::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    stop();
}

void RpcService::start()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (gatewayThread_ == nullptr) {
        gatewayThread_ = new QThread();
        QObject::connect(gatewayThread_, &QThread::started, this, &RpcService::onGatewayThreadStarted, Qt::DirectConnection);
        gatewayThread_->start();
    }
}

void RpcService::stop()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (gatewayThread_ != nullptr) {
        // NOTE(hege):关闭queue，让rpc结束和rpc线程退出=
        // servicemgr::shutdown里面rpcservice线程第一个退是必须的，这样下面这个任务才可以抛到pushservice线程=
        QMetaObject::invokeMethod(g_sm->pushService(), "onGatewayClosed", Qt::QueuedConnection);

        grpcServer_->Shutdown();
        grpcServer_ = nullptr;

        gatewayThread_->quit();
        gatewayThread_->wait();
        delete gatewayThread_;
        gatewayThread_ = nullptr;
    }
}

void RpcService::onGatewayThreadStarted()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    std::string server_address("0.0.0.0:50051");
    Gateway gateway("btgateway");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&gateway);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfLog(QString("gateway listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfLog(QString("gateway shutdown"));
}
