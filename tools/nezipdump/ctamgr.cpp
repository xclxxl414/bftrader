#include "ctamgr.h"
#include "servicemgr.h"

CtaMgr::CtaMgr(QObject* parent)
    : QObject(parent)
{
}

void CtaMgr::init()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::FLOGIC);
}

void CtaMgr::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::FLOGIC);
}
