#include "dbservice.h"
#include "servicemgr.h"

DbService::DbService(QObject* parent)
    : QObject(parent)
{
}

void DbService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);
}

void DbService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::DB);
}
