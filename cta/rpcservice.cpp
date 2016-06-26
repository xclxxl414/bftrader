#include "rpcservice.h"
#include "bfcta.grpc.pb.h"
#include "dbservice.h"
#include "pushservice.h"
#include "safequeue.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bfcta;

//
// cta的rpc可以直接调用gatewaymgr的slots以调用gateway，grpc是多线程安全的=
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
    virtual ::grpc::Status ConnectPush(::grpc::ServerContext* context, const BfConnectPushReq* request, ::grpc::ServerWriter< ::google::protobuf::Any>* writer) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        QString clientId = request->clientid().c_str();
        BfDebug("(%s)->Connect", qPrintable(clientId));

        // check modelid
        QString modelId = g_sm->dbService()->getModelId(clientId);
        if (modelId.length() == 0 || modelId != request->strparam().c_str()) {
            BfDebug("(%s)->Connect,invalid param: modelId!", qPrintable(clientId));
            return grpc::Status::OK;
        }

        // push now
        auto queue = new SafeQueue<google::protobuf::Any>;
        QMetaObject::invokeMethod(g_sm->pushService(), "connectClient", Qt::QueuedConnection, Q_ARG(QString, ctaId_), Q_ARG(BfConnectPushReq, *request), Q_ARG(void*, (void*)queue));
        while (auto data = queue->dequeue()) {
            // NOTE(hege):客户端异常导致stream关闭
            bool ok = writer->Write(*data);
            delete data;
            if (!ok) {
                BfDebug("(%s)-->stream closed!", qPrintable(clientId));
                QMetaObject::invokeMethod(g_sm->pushService(), "disconnectClient", Qt::QueuedConnection, Q_ARG(QString, clientId));
                break;
            }
        }

        BfDebug("(%s)->Connect exit!", qPrintable(clientId));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const BfPingData* request, BfPingData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        response->set_message(request->message());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status DisconnectPush(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfDebug("(%s)->Disconnect", qPrintable(clientId));

        // NOTE(hege):关闭stream
        QMetaObject::invokeMethod(g_sm->pushService(), "disconnectClient", Qt::QueuedConnection, Q_ARG(QString, clientId));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status GetRobotInfo(::grpc::ServerContext* context, const BfKvData* request, BfKvData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const BfSendOrderReq* request, BfSendOrderResp* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        BfDebug("clientId=%s", qPrintable(clientId));

        QString gatewayId = g_sm->dbService()->getGatewayId(clientId);
        g_sm->gatewayMgr()->sendOrder(gatewayId, *request, *response);

        // update map
        if (response->bforderid().length() != 0) {
            g_sm->dbService()->putOrderEx(clientId, response->bforderid().c_str());
        }

        return grpc::Status::OK;
    }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const BfCancelOrderReq* request, BfVoid* response) override
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

    std::string server_address("0.0.0.0:50053");
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
