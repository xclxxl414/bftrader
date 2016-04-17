#include "rpcservice.h"
#include "bfgateway.grpc.pb.h"
#include "logger.h"
#include "servicemgr.h"
#include <QThread>
#include <QtCore/QDebug>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfgateway;

// 策略间是完全隔离的，gateway在connect时候分配robot线程
// robot对象，将robot对象移动到robot线程，并记录<peer,robotobj>
// map<peer,robotobj> 用于分离rpc命令,servercontext::get_peer
// map<reqid,robotobj> 用于分离请求的回报
// map<orderid,robotobj> 用于分离委托推送
// map<orderref,orderid> 分离撮合平台委托回报比如CTP
// map<sysid,orderid> 分离交易所委托回报
// map<tradeid,robotobj> 用于分离成交推送
// map<symbol,robotobjlist> 用于分离行情推送
// gateway提供界面添加策略，指定名称/robotid，状态:离线/在线
// robotid作为主键，将记录持仓 挂单/委托 成交等信息到数据库，便于统计
// 策略本身也要维护自己的持仓 挂单 委托 成交=
class BfGateway final : public BfGatewayService::Service {
public:
    BfGateway() {}
    virtual ~BfGateway() {}
    virtual ::grpc::Status Connect(::grpc::ServerContext* context, const ::bftrader::BfConnectReq* request, ::bftrader::BfConnectResp* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status SetKv(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetKv(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfKvData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetContract(::grpc::ServerContext* context, const ::bftrader::BfGetContractReq* request, ::bftrader::BfContractData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetContractList(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::grpc::ServerWriter< ::bftrader::BfContractData>* writer) override { return grpc::Status::OK; }
    virtual ::grpc::Status Subscribe(::grpc::ServerContext* context, const ::bftrader::BfSubscribeReq* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const ::bftrader::BfOrderReq* request, ::bftrader::BfOrderResp* response) override { return grpc::Status::OK; }
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
