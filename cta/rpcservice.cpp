#include "rpcservice.h"
#include "bfcta.grpc.pb.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfcta;

//
// Cta，需要实现异步服务器，一些api/rpc调用并不能在当前线程下完成，rpcservice的api调用线程是不确定的=
// 可以把rpc调用全部转移到逻辑线程上去做，做完了统一完成=
//
class Cta final : public BfCtaService::Service {
public:
    Cta()
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
        return grpc::Status::OK;
    }
    virtual ::grpc::Status CancelOrder(::grpc::ServerContext* context, const ::bftrader::BfCancelOrderReq* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status Disconnect(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
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
}
