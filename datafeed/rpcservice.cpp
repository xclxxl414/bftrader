#include "rpcservice.h"
#include "bfdatafeed.grpc.pb.h"
#include "dbservice.h"
#include "servicemgr.h"
#include <QThread>
#include <grpc++/grpc++.h>

//
// Datafeed
//

class Datafeed final : public BfDatafeedService::Service {
public:
    Datafeed()
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Datafeed()
    {
        BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }

    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const BfPingData* request, BfPingData* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        response->set_message(request->message());
        return grpc::Status::OK;
    }

    virtual ::grpc::Status InsertTick(::grpc::ServerContext* context, const BfTickData* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "insertTick", Qt::QueuedConnection, Q_ARG(BfTickData, *request));
        return grpc::Status::OK;
    }

    virtual ::grpc::Status InsertBar(::grpc::ServerContext* context, const BfBarData* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "insertBar", Qt::QueuedConnection, Q_ARG(BfBarData, *request));
        return grpc::Status::OK;
    }

    virtual ::grpc::Status InsertContract(::grpc::ServerContext* context, const BfContractData* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "insertContract", Qt::QueuedConnection, Q_ARG(BfContractData, *request));
        return grpc::Status::OK;
    }

    virtual ::grpc::Status GetTick(::grpc::ServerContext* context, const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);
        g_sm->dbService()->getTick(request, writer);
        return grpc::Status::OK;
    }

    virtual ::grpc::Status GetBar(::grpc::ServerContext* context, const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        g_sm->dbService()->getBar(request, writer);
        return grpc::Status::OK;
    }

    virtual ::grpc::Status GetContract(::grpc::ServerContext* context, const BfGetContractReq* request, ::grpc::ServerWriter<BfContractData>* writer) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        g_sm->dbService()->getContract(request, writer);
        return grpc::Status::OK;
    }

    virtual ::grpc::Status DeleteTick(::grpc::ServerContext* context, const BfDeleteTickReq* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "deleteTick", Qt::QueuedConnection, Q_ARG(BfDeleteTickReq, *request));
        return grpc::Status::OK;
    }

    virtual ::grpc::Status DeleteBar(::grpc::ServerContext* context, const BfDeleteBarReq* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "deleteBar", Qt::QueuedConnection, Q_ARG(BfDeleteBarReq, *request));
        return grpc::Status::OK;
    }

    virtual ::grpc::Status DeleteContract(::grpc::ServerContext* context, const BfDeleteContractReq* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "deleteContract", Qt::QueuedConnection, Q_ARG(BfDeleteContractReq, *request));
        return grpc::Status::OK;
    }
    virtual ::grpc::Status CleanAll(::grpc::ServerContext* context, const BfVoid* request, BfVoid* response) override
    {
        //BfLog("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());

        QString clientId = getClientId(context);

        QMetaObject::invokeMethod(g_sm->dbService(), "cleanAll", Qt::QueuedConnection);
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

    stop();
}

void RpcService::start()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (datafeedThread_ == nullptr) {
        datafeedThread_ = new QThread();
        QObject::connect(datafeedThread_, &QThread::started, this, &RpcService::onDatafeedThreadStarted, Qt::DirectConnection);
        datafeedThread_->start();
    }
}

void RpcService::stop()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (datafeedThread_ != nullptr) {
        grpcServer_->Shutdown();
        grpcServer_ = nullptr;

        datafeedThread_->quit();
        datafeedThread_->wait();
        delete datafeedThread_;
        datafeedThread_ = nullptr;
    }
}

void RpcService::onDatafeedThreadStarted()
{
    BfLog(__FUNCTION__);
    if (g_sm->isCurrentOn(ServiceMgr::RPC)) {
        qFatal("g_sm->CurrentOn(ServiceMgr::RPC)");
    }

    std::string server_address("0.0.0.0:50052");
    Datafeed datafeed;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&datafeed);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfLog(QString("datafeed listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfLog(QString("datafeed shutdown"));
}
