#include "rpcservice.h"
#include "logger.h"
#include "servicemgr.h"
#include <QtCore/QDebug>

RpcService::RpcService(QObject* parent)
    : QObject(parent)
{
}

void RpcService::init()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}

void RpcService::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);
}
