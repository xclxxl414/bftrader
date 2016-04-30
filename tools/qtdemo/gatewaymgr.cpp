#include "gatewaymgr.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "logger.h"
#include "servicemgr.h"

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

void GatewayMgr::showVersion()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfInfo(QString("mdapi version: ") + CThostFtdcMdApi::GetApiVersion());
    BfInfo(QString("tdapi version: ") + CThostFtdcTraderApi::GetApiVersion());
}
