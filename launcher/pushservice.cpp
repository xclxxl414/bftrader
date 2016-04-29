#include "pushservice.h"
#include "servicemgr.h"

PushService::PushService(QObject* parent)
    : QObject(parent)
{
}

void PushService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}
