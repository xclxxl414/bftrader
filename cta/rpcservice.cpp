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
    explicit Cta()
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Cta()
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ::grpc::Status ConnectPush(::grpc::ServerContext* context, const BfConnectPushReq* request, ::grpc::ServerWriter< ::google::protobuf::Any>* writer) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        QString clientId = request->clientid().c_str();
        BfLog("(%s)->Connect", qPrintable(clientId));

        // push now
        auto queue = new SafeQueue<google::protobuf::Any>;
        QMetaObject::invokeMethod(g_sm->pushService(), "connectClient", Qt::QueuedConnection, Q_ARG(BfConnectPushReq, *request), Q_ARG(void*, (void*)queue));
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
    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const BfPingData* request, BfPingData* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        response->set_message(request->message());
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
    //TODO(hege):do it
    virtual ::grpc::Status Start(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    //TODO(hege):do it
    virtual ::grpc::Status Stop(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
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
}

void RpcService::start()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (ctaThread_ == nullptr) {
        ctaThread_ = new QThread();
        QObject::connect(ctaThread_, &QThread::started, this, &RpcService::onCtaThreadStarted, Qt::DirectConnection);
        ctaThread_->start();
    }
}

void RpcService::stop()
{
    BfLog(__FUNCTION__);
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
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    std::string server_address("0.0.0.0:50053");
    Cta cta;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&cta);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfLog(QString("cta listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfLog(QString("cta shutdown"));
}
