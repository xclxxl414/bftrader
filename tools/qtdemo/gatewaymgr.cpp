#include "gatewaymgr.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "servicemgr.h"

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::BLOGIC);
}

void GatewayMgr::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::BLOGIC);
}

void GatewayMgr::showVersion()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::BLOGIC);

    BfLog(QString("mdapi version: ") + CThostFtdcMdApi::GetApiVersion());
    BfLog(QString("tdapi version: ") + CThostFtdcTraderApi::GetApiVersion());
}
