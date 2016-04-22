#include "ctpmgr.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "logger.h"
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

void CtpMgr::showVersion()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfInfo(QString("mdapi version: ") + CThostFtdcMdApi::GetApiVersion());
    BfInfo(QString("tdapi version: ") + CThostFtdcTraderApi::GetApiVersion());
}
