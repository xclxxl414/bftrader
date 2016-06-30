#include "pushservice.h"
#include "servicemgr.h"

PushService::PushService(QObject* parent)
    : QObject(parent)
{
}

void PushService::init()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}

void PushService::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}
