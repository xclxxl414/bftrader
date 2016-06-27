#include "gatewaymgr.h"
#include "servicemgr.h"
#include <QCoreApplication>
#include <QDir>

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
