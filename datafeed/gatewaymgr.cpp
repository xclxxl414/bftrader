#include "gatewaymgr.h"
#include "servicemgr.h"

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::BLOGIC);

    // qRegisterMetaType
    qRegisterMetaType<BfTickData>("BfTickData");
    qRegisterMetaType<BfBarData>("BfBarData");
    qRegisterMetaType<BfContractData>("BfContractData");

    qRegisterMetaType<BfGetTickReq>("BfGetTickReq");
    qRegisterMetaType<BfGetBarReq>("BfGetBarReq");
    qRegisterMetaType<BfGetContractReq>("BfGetContractReq");

    qRegisterMetaType<BfDeleteTickReq>("BfDeleteTickReq");
    qRegisterMetaType<BfDeleteBarReq>("BfDeleteBarReq");
    qRegisterMetaType<BfDeleteContractReq>("BfDeleteContractReq");
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::BLOGIC);
}
