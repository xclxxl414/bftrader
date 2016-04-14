#include "rpcservice.h"

#include <QtCore/QDebug>

#include "servicemgr.h"
#include "logger.h"

RpcService::RpcService(QObject *parent) : QObject(parent)
{

}

void RpcService::init(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

}

void RpcService::shutdown(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::RPC);

}
