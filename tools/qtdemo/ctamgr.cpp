#include "ctamgr.h"
#include "servicemgr.h"
#include "ztalib.h"

CtaMgr::CtaMgr(QObject* parent)
    : QObject(parent)
{
}

void CtaMgr::init()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::FLOGIC);

    //talib
    Z_Initialize();
}

void CtaMgr::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::FLOGIC);

    //talib
    Z_Shutdown();
}
