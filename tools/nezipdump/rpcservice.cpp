#include "rpcservice.h"
#include "servicemgr.h"
#include <QtCore/QDebug>

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
