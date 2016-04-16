#include "rpcservice.h"
#include "bfgateway.grpc.pb.h"
#include "logger.h"
#include "servicemgr.h"
#include <QThread>
#include <QtCore/QDebug>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfgateway;

class BfGateway final : public BfGatewayService::Service {
public:
    BfGateway() {}
    virtual ~BfGateway() {}
    virtual ::grpc::Status Connect(::grpc::ServerContext* context, const ::bftrader::BfConnectReq* request, ::bftrader::BfErrorData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status SetKv(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetKv(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfKvData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetContract(::grpc::ServerContext* context, const ::bftrader::BfGetContractReq* request, ::bftrader::BfContractData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetContractList(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::grpc::ServerWriter< ::bftrader::BfContractData>* writer) override { return grpc::Status::OK; }
    virtual ::grpc::Status Subscribe(::grpc::ServerContext* context, const ::bftrader::BfSubscribeReq* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const ::bftrader::BfOrderReq* request, ::bftrader::BfOrderResponse* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const ::bftrader::BfCancelOrderReq* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status QueryAccount(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status QueryPosition(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status Close(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
};

RpcService::RpcService(QObject* parent)
    : QObject(parent)
{
}

void RpcService::init()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    gatewayThread_ = new QThread();
    QObject::connect(gatewayThread_, &QThread::started, this, &RpcService::onGatewayThreadStarted, Qt::DirectConnection);
    gatewayThread_->start();
}

void RpcService::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

    grpcServer_->Shutdown();
    grpcServer_ = nullptr;

    gatewayThread_->quit();
    gatewayThread_->wait();
    delete gatewayThread_;
    gatewayThread_ = nullptr;
}

void RpcService::onGatewayThreadStarted()
{
    g_sm->logger()->info(__FUNCTION__);
    if(g_sm->isCurrentOn(ServiceMgr::RPC)){
        qFatal("g_sm->CurrentOn(ServiceMgr::RPC)");
    }

    std::string server_address("0.0.0.0:50051");
    BfGateway gateway;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&gateway);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    g_sm->logger()->info(QString("gateway listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    g_sm->logger()->info(QString("gateway shutdown"));
}
