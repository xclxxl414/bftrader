#include "gatewaymgr.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "logger.h"
#include "servicemgr.h"
#include "ztalib.h"
GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    //talib
    Z_Initialize();
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    //talib
    Z_Shutdown();
}

void GatewayMgr::showVersion()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfLog(QString("mdapi version: ") + CThostFtdcMdApi::GetApiVersion());
    BfLog(QString("tdapi version: ") + CThostFtdcTraderApi::GetApiVersion());
}
