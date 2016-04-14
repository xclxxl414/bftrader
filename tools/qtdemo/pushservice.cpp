#include "pushservice.h"

#include "servicemgr.h"
#include "logger.h"


PushService::PushService(QObject *parent) : QObject(parent)
{

}

void PushService::init(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

}

void PushService::shutdown(){
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

}
