#include "ctpmgr.h"
#include "servicemgr.h"
#include "logger.h"
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcMdApi.h"

CtpMgr::CtpMgr(QObject *parent) : QObject(parent)
{

}

void CtpMgr::init(){
    g_sm->checkCurrentOn(ServiceMgr::CTP);
}

void CtpMgr::shutdown(){
    g_sm->checkCurrentOn(ServiceMgr::CTP);
}

void CtpMgr::showVersion(){
    g_sm->checkCurrentOn(ServiceMgr::CTP);

    g_sm->logger()->info(QString("mdapi version: ") + CThostFtdcMdApi::GetApiVersion());
    g_sm->logger()->info(QString("tdapi version: ") + CThostFtdcTraderApi::GetApiVersion());
}

