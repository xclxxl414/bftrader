#include "rpcservice.h"
#include "bfdatafeed.grpc.pb.h"
//#include "ctp_utils.h"
//#include "ctpmgr.h"
//#include "encode_utils.h"
//#include "pushservice.h"
#include "servicemgr.h"
#include <QThread>
//#include <QtCore/QDebug>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfdatafeed;

//
// Gateway
// 全广播机制
//

class Datafeed final : public BfDatafeedService::Service {
public:
    Datafeed()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Datafeed()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }

    virtual ::grpc::Status Ping(::grpc::ServerContext* context, const ::bftrader::BfPingData* request, ::bftrader::BfPingData* response) override
    {
        QString clientId = getClientId(context);
        response->set_message(request->message());
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

    if (datafeedThread_ == nullptr) {
        datafeedThread_ = new QThread();
        QObject::connect(datafeedThread_, &QThread::started, this, &RpcService::onDatafeedThreadStarted, Qt::DirectConnection);
        datafeedThread_->start();
    }
}

void RpcService::stop()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    if (datafeedThread_ != nullptr) {
        grpcServer_->Shutdown();
        grpcServer_ = nullptr;

        datafeedThread_->quit();
        datafeedThread_->wait();
        delete datafeedThread_;
        datafeedThread_ = nullptr;

        //QMetaObject::invokeMethod(g_sm->pushService(), "onGatewayClose", Qt::QueuedConnection);
    }
}

void RpcService::onDatafeedThreadStarted()
{
    BfDebug(__FUNCTION__);
    if (g_sm->isCurrentOn(ServiceMgr::RPC)) {
        qFatal("g_sm->CurrentOn(ServiceMgr::RPC)");
    }

    std::string server_address("0.0.0.0:50052");
    Datafeed datafeed;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&datafeed);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BfInfo(QString("datafeed listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    BfInfo(QString("datafeed shutdown"));
}
