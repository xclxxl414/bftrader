#include "ctpmgr.h"
#include "servicemgr.h"

CtpMgr::CtpMgr(QObject* parent)
    : QObject(parent)
{
}

void CtpMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // qRegisterMetaType
    qRegisterMetaType<BfTickData>("BfTickData");
    qRegisterMetaType<BfBarData>("BfBarData");
    qRegisterMetaType<BfContractData>("BfContractData");

    qRegisterMetaType<BfDeleteTickReq>("BfDeleteTickReq");
    qRegisterMetaType<BfDeleteBarReq>("BfDeleteBarReq");
    qRegisterMetaType<BfDatafeedDeleteContractReq>("BfDatafeedDeleteContractReq");
}

void CtpMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
