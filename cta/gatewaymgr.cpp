#include "gatewaymgr.h"
#include "bfproxy.grpc.pb.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfproxy;

//
// Proxy
//

class Proxy final : public BfProxyService::Service {
public:
    Proxy()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ~Proxy()
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
    }
    virtual ::grpc::Status OnTradeWillBegin(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnGotContracts(::grpc::ServerContext* context, const ::bftrader::BfVoid* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnPing(::grpc::ServerContext* context, const ::bftrader::BfPingData* request, ::bftrader::BfPingData* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnTick(::grpc::ServerContext* context, const ::bftrader::BfTickData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnError(::grpc::ServerContext* context, const ::bftrader::BfErrorData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnLog(::grpc::ServerContext* context, const ::bftrader::BfLogData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnTrade(::grpc::ServerContext* context, const ::bftrader::BfTradeData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnOrder(::grpc::ServerContext* context, const ::bftrader::BfOrderData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnPosition(::grpc::ServerContext* context, const ::bftrader::BfPositionData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
    virtual ::grpc::Status OnAccount(::grpc::ServerContext* context, const ::bftrader::BfAccountData* request, ::bftrader::BfVoid* response) override
    {
        BfDebug("%s on thread:%d", __FUNCTION__, ::GetCurrentThreadId());
        return grpc::Status::OK;
    }
};

//
// GatewayMgr
//
GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
