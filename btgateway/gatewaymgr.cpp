#include "gatewaymgr.h"
#include "logger.h"
#include "servicemgr.h"

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // qRegisterMetaType
    qRegisterMetaType<BfAccountData>("BfAccountData");
    qRegisterMetaType<BfPositionData>("BfPositionData");
    qRegisterMetaType<BfOrderData>("BfOrderData");
    qRegisterMetaType<BfTradeData>("BfTradeData");
    qRegisterMetaType<BfNotificationData>("BfNotificationData");
    qRegisterMetaType<BfContractData>("BfContractData");
    qRegisterMetaType<BfErrorData>("BfErrorData");
    qRegisterMetaType<BfLogData>("BfLogData");

    qRegisterMetaType<BfConnectPushReq>("BfConnectPushReq");
    qRegisterMetaType<BfGetContractReq>("BfGetContractReq");
    qRegisterMetaType<BfSendOrderReq>("BfSendOrderReq");
    qRegisterMetaType<BfCancelOrderReq>("BfCancelOrderReq");
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

int GatewayMgr::getOrderId()
{
    orderId_++;
    return orderId_;
}

void GatewayMgr::resetData()
{
    // orderid
    orderId_ = 0;

    // tradeid
    tradeId_ = 0;

    // account
    account_.Clear();
    account_.set_accountid("HegeV5");
    account_.set_balance(1000000.00);
    account_.set_available(1000000.00);
    account_.set_frozenmargin(0.0);
    account_.set_closeprofit(0.0);
    account_.set_positionprofit(0.0);

    //position
    for (auto pos : positions_) {
        delete pos;
    }
    positions_.clear();

    //order
    for (auto order : orders_) {
        delete order;
    }
    orders_.clear();

    //trade
    for (auto trade : trades_) {
        delete trade;
    }
    trades_.clear();

    //symbols
    symbol_all_.clear();
    symbol_my_.clear();
}

QString GatewayMgr::genOrderId()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);

    int orderId = getOrderId();
    QString bfOrderId = QString().sprintf("%d", orderId);
    return bfOrderId;
}

void GatewayMgr::start()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    resetData();
    emit this->tradeWillBegin();
    emit this->gotContracts(symbol_my_, symbol_all_);
}

void GatewayMgr::stop()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    resetData();
}

void GatewayMgr::queryAccount()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    emit this->gotAccount(account_);
}

void GatewayMgr::queryPosition()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfNotificationData note;
    note.set_type(NOTIFICATION_BEGINQUERYPOSITION);
    emit this->gotNotification(note);

    for (auto pos : positions_) {
        emit this->gotPosition(*pos);
    }

    note.set_type(NOTIFICATION_ENDQUERYPOSITION);
    emit this->gotNotification(note);
}

void GatewayMgr::queryOrders()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfNotificationData note;
    note.set_type(NOTIFICATION_BEGINQUERYORDERS);
    emit this->gotNotification(note);

    for (auto order : orders_) {
        emit this->gotOrder(*order);
    }

    note.set_type(NOTIFICATION_ENDQUERYORDERS);
    emit this->gotNotification(note);
}

void GatewayMgr::sendOrder(const BfSendOrderReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    QString bfOrderId = genOrderId();
    sendOrderWithId(bfOrderId, req);
}

//TODO(hege): do it
void GatewayMgr::sendOrderWithId(QString byOrderId, const BfSendOrderReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

//TODO(hege): do it
void GatewayMgr::cancelOrder(const BfCancelOrderReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}
