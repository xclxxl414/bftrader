#include "ctpmgr.h"
#include "servicemgr.h"

CtpMgr::CtpMgr(QObject* parent)
    : QObject(parent)
{
}

void CtpMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

void CtpMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
