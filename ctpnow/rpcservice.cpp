#include "rpcservice.h"
#include "bfgateway.grpc.pb.h"
#include "logger.h"
#include "pushservice.h"
#include "servicemgr.h"
#include <QThread>
#include <QtCore/QDebug>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfgateway;

//
// Gateway
//

//
// 首先实现一个最简单的版本：全广播机制=
//
// TODO(hege):实现隔离机制
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
//
class Gateway final : public BfGatewayService::Service {
public:
    Gateway() {}
    virtual ~Gateway() {}
    virtual ::grpc::Status Connect(::grpc::ServerContext* context, const ::bftrader::BfConnectReq* request, ::bftrader::BfConnectResp* response) override
    {
        g_sm->logger()->info(__FUNCTION__);

        QString peer = context->peer().c_str();
        QString robotId = request->robotid().c_str();
        QString robotIp = request->robotip().c_str();
        qint32 robotPort = request->robotport();
        g_sm->logger()->info(QString().sprintf("peer:%s,%s:%s:%d", context->peer().c_str(), request->robotid().c_str(), request->robotip().c_str(), request->robotport()));
        QMetaObject::invokeMethod(g_sm->pushService(), "onRobotConnected", Qt::QueuedConnection, Q_ARG(QString, robotId), Q_ARG(QString, robotIp), Q_ARG(qint32, robotPort));

        response->set_exchangeopened(true);
        return grpc::Status::OK;
    }
    virtual ::grpc::Status SetKv(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetKv(::grpc::ServerContext* context, const ::bftrader::BfKvData* request, ::bftrader::BfKvData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetContract(::grpc::ServerContext* context, const ::bftrader::BfGetContractReq* request, ::bftrader::BfContractData* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status GetContractList(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::grpc::ServerWriter< ::bftrader::BfContractData>* writer) override { return grpc::Status::OK; }
    virtual ::grpc::Status Subscribe(::grpc::ServerContext* context, const ::bftrader::BfSubscribeReq* request, ::bftrader::BfVoid* response) override
    {
        g_sm->logger()->info(__FUNCTION__);

        // metadata-key只能是小写的=
        if (0 == context->client_metadata().count("robotid")) {
            g_sm->logger()->info("no matadata: robotid");
        } else {
            auto its = context->client_metadata().equal_range("robotid");
            auto it = its.first;
            grpc::string robotId = grpc::string(it->second.begin(), it->second.end());
            g_sm->logger()->info(QString().sprintf("metadata: robotid=%s", robotId.c_str()));
        }
        return grpc::Status::OK;
    }
    virtual ::grpc::Status SendOrder(::grpc::ServerContext* context, const ::bftrader::BfSendOrderReq* request, ::bftrader::BfSendOrderResp* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const ::bftrader::BfCancelOrderReq* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status QueryAccount(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status QueryPosition(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
    virtual ::grpc::Status Close(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override { return grpc::Status::OK; }
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
    if (g_sm->isCurrentOn(ServiceMgr::RPC)) {
        qFatal("g_sm->CurrentOn(ServiceMgr::RPC)");
    }

    std::string server_address("0.0.0.0:50051");
    Gateway gateway;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&gateway);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    g_sm->logger()->info(QString("gateway listening on ") + server_address.c_str());
    grpcServer_ = server.get();

    server->Wait();
    g_sm->logger()->info(QString("gateway shutdown"));
}
