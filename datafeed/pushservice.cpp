#include "pushservice.h"

#include "logger.h"
#include "servicemgr.h"

PushService::PushService(QObject* parent)
    : QObject(parent)
{
}

void PushService::init()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}

void PushService::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}
