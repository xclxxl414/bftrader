#include "gatewaymgr.h"
#include "dbservice.h"
#include "logger.h"
#include "servicemgr.h"
#include <QDateTime>

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    BfLog(__FUNCTION__);
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
    qRegisterMetaType<BfTickData>("BfTickData");

    qRegisterMetaType<BfConnectPushReq>("BfConnectPushReq");
    qRegisterMetaType<BfGetContractReq>("BfGetContractReq");
    qRegisterMetaType<BfSendOrderReq>("BfSendOrderReq");
    qRegisterMetaType<BfCancelOrderReq>("BfCancelOrderReq");
    qRegisterMetaType<BfGetTickReq>("BfGetTickReq");

    // dbservice
    QObject::connect(g_sm->dbService(), &DbService::gotTick, this, &GatewayMgr::onGotTick);
}

void GatewayMgr::shutdown()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

int GatewayMgr::getOrderId()
{
    orderId_++;
    return orderId_;
}

int GatewayMgr::getTradeId()
{
    tradeId_++;
    return tradeId_;
}

QString GatewayMgr::genTradeId()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    int tradeId = getTradeId();
    QString bfTradeId = QString().sprintf("%d", tradeId);
    return bfTradeId;
}

void GatewayMgr::resetData()
{
    // orderid
    orderId_ = 0;

    // tradeid
    tradeId_ = 0;

    // account
    account_.Clear();
    account_.set_accountid("demo");
    account_.set_balance(100000000.00);
    account_.set_available(100000000.00);
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
}

QString GatewayMgr::genOrderId()
{
    BfLog(__FUNCTION__);
    if (!g_sm->isCurrentOn(ServiceMgr::LOGIC)) {
        g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    }

    int orderId = getOrderId();
    QString bfOrderId = QString().sprintf("%d", orderId);
    return bfOrderId;
}

void GatewayMgr::start(const BfGetTickReq& req)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
    BfLog("tradeWillBegin:(%s.%s:%s %s-%s %s)",
        req.symbol().c_str(), req.exchange().c_str(),
        req.fromdate().c_str(), req.fromtime().c_str(),
        req.todate().c_str(), req.totime().c_str());

    resetData();
    emit this->tradeWillBegin(req);

    //init long pos
    BfPositionData* longPos = new BfPositionData();
    QString longKey = QString().sprintf("%s-%s-long", req.symbol().c_str(), req.exchange().c_str());
    positions_.insert(longKey, longPos);

    longPos->set_symbol(req.symbol());
    longPos->set_exchange(req.exchange());
    longPos->set_direction(DIRECTION_LONG);
    longPos->set_position(0);
    longPos->set_price(0);
    longPos->set_frozen(0);
    longPos->set_ydposition(0);

    //init short pos
    BfPositionData* shortPos = new BfPositionData();
    QString shortKey = QString().sprintf("%s-%s-short", req.symbol().c_str(), req.exchange().c_str());
    positions_.insert(shortKey, shortPos);

    shortPos->set_symbol(req.symbol());
    shortPos->set_exchange(req.exchange());
    shortPos->set_direction(DIRECTION_SHORT);
    shortPos->set_position(0);
    shortPos->set_price(0);
    shortPos->set_frozen(0);
    shortPos->set_ydposition(0);
}

void GatewayMgr::stop()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    resetData();
    emit this->tradeStopped();
}

void GatewayMgr::queryAccount()
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    emit this->gotAccount(account_);
}

void GatewayMgr::queryPosition()
{
    BfLog(__FUNCTION__);
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
    BfLog(__FUNCTION__);
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
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    QString bfOrderId = genOrderId();
    sendOrderWithId(bfOrderId, req);
}

void GatewayMgr::sendOrderWithId(QString bfOrderId, const BfSendOrderReq& req)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfOrderData* order = new BfOrderData();
    orders_.insert(bfOrderId, order);

    // 保存代码和报单号=
    order->set_exchange(req.exchange());
    order->set_symbol(req.symbol());
    order->set_bforderid(bfOrderId.toStdString());
    order->set_direction(req.direction()); //方向=
    order->set_offset(req.offset()); //开平=
    order->set_status(STATUS_NOTTRADED); //状态=

    //价格、报单量等数值=
    order->set_price(req.price());
    order->set_totalvolume(req.volume());
    order->set_tradedvolume(0);
    order->set_insertdate(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString());
    order->set_inserttime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
    order->set_canceltime("");

    do {
        //只支持固定的品种=
        if (1) {
            QString longKey = QString().sprintf("%s-%s-long", order->symbol().c_str(), order->exchange().c_str());
            if (!positions_.contains(longKey)) {
                BfLog("only support the symbol");
                order->set_status(STATUS_CANCELLED); //状态=
                order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
                break;
            }
        }
        //只支持限价单=
        if (req.pricetype() != PRICETYPE_LIMITPRICE) {
            BfLog("only support limitprice");
            order->set_status(STATUS_CANCELLED); //状态=
            order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
            break;
        }
        //只支持1-500手=
        if (req.volume() < 1 || req.volume() > 500) {
            BfLog("only support volume: (1-500)");
            order->set_status(STATUS_CANCELLED); //状态=
            order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
            break;
        }
        //只支持平今=
        if (req.offset() != OFFSET_OPEN && req.offset() != OFFSET_CLOSE) {
            BfLog("only support open close");
            order->set_status(STATUS_CANCELLED); //状态=
            order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
            break;
        }
        //仓位不够空平=
        if (req.offset() == OFFSET_CLOSE && req.direction() == DIRECTION_SHORT) {
            QString longKey = QString().sprintf("%s-%s-long", order->symbol().c_str(), order->exchange().c_str());
            BfPositionData* longPos = positions_.value(longKey);
            if (longPos->position() < order->totalvolume()) {
                BfLog("no enough position for short+close");
                order->set_status(STATUS_CANCELLED); //状态=
                order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
                break;
            }
        }
        //仓位不够多平
        if (req.offset() == OFFSET_CLOSE && (req.direction() == DIRECTION_LONG || req.direction() == DIRECTION_NET)) {
            QString shortKey = QString().sprintf("%s-%s-short", order->symbol().c_str(), order->exchange().c_str());
            BfPositionData* shortPos = positions_.value(shortKey);
            if (shortPos->position() < order->totalvolume()) {
                BfLog("no enough position for long+close");
                order->set_status(STATUS_CANCELLED); //状态=
                order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
                break;
            }
        }
    } while (0);

    // frozen
    if (order->status() == STATUS_NOTTRADED) {
        if (order->offset() == OFFSET_OPEN && order->direction() == DIRECTION_SHORT) {
            QString shortKey = QString().sprintf("%s-%s-short", order->symbol().c_str(), order->exchange().c_str());
            BfPositionData* shortPos = positions_.value(shortKey);
            shortPos->set_frozen(shortPos->frozen() + order->totalvolume());
        }
        if (order->offset() == OFFSET_OPEN && (order->direction() == DIRECTION_LONG || order->direction() == DIRECTION_NET)) {
            QString longKey = QString().sprintf("%s-%s-long", order->symbol().c_str(), order->exchange().c_str());
            BfPositionData* longPos = positions_.value(longKey);
            longPos->set_frozen(longPos->frozen() + order->totalvolume());
        }
    }

    // dispatch
    emit this->gotOrder(*order);
}

void GatewayMgr::cancelOrder(const BfCancelOrderReq& req)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    QString bfOrderId = req.bforderid().c_str();

    if (!orders_.contains(bfOrderId)) {
        BfLog("no order: (%s)", qPrintable(bfOrderId));
        return;
    }

    BfOrderData* order = orders_.value(bfOrderId);
    if (order->status() == STATUS_ALLTRADED || order->status() == STATUS_CANCELLED) {
        BfLog("order complete yet:(%s)", qPrintable(bfOrderId));
        return;
    }

    // frozen
    if (order->status() == STATUS_NOTTRADED) {
        if (order->offset() == OFFSET_OPEN && order->direction() == DIRECTION_SHORT) {
            QString shortKey = QString().sprintf("%s-%s-short", order->symbol().c_str(), order->exchange().c_str());
            BfPositionData* shortPos = positions_.value(shortKey);
            shortPos->set_frozen(shortPos->frozen() - order->totalvolume());
        }
        if (order->offset() == OFFSET_OPEN && (order->direction() == DIRECTION_LONG || order->direction() == DIRECTION_NET)) {
            QString longKey = QString().sprintf("%s-%s-long", order->symbol().c_str(), order->exchange().c_str());
            BfPositionData* longPos = positions_.value(longKey);
            longPos->set_frozen(longPos->frozen() - order->totalvolume());
        }
    }

    // cancel
    order->set_status(STATUS_CANCELLED);
    order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());

    emit this->gotOrder(*order);
}

void GatewayMgr::onGotTick(const BfTickData& tick)
{
    //BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    for (auto order : orders_) {
        if (order->status() == STATUS_ALLTRADED || order->status() == STATUS_CANCELLED) {
            continue;
        }
        if (order->symbol() != tick.symbol() || order->exchange() != tick.exchange()) {
            continue;
        }

        //多开
        if (order->direction() == DIRECTION_LONG || order->direction() == DIRECTION_NET) {
            if (order->offset() == OFFSET_OPEN) {
                if (order->price() >= tick.lastprice()) {
                    this->onLongOpen(order, tick);
                }
            }
        }
        //空开
        if (order->direction() == DIRECTION_SHORT) {
            if (order->offset() == OFFSET_OPEN) {
                if (order->price() <= tick.lastprice()) {
                    this->onShortOpen(order, tick);
                }
            }
        }
        //多平
        if (order->direction() == DIRECTION_LONG || order->direction() == DIRECTION_NET) {
            if (order->offset() == OFFSET_CLOSE) {
                if (order->price() >= tick.lastprice()) {
                    this->onLongClose(order, tick);
                }
            }
        }
        //空平
        if (order->direction() == DIRECTION_SHORT) {
            if (order->offset() == OFFSET_CLOSE) {
                if (order->price() <= tick.lastprice()) {
                    this->onShortClose(order, tick);
                }
            }
        }
    }
}

void GatewayMgr::onLongOpen(BfOrderData* order, const BfTickData& tick)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // ==order==
    order->set_tradedvolume(order->totalvolume());
    order->set_status(STATUS_ALLTRADED);

    // ==trade==
    BfTradeData* trade = new BfTradeData();
    QString tradeId = genTradeId();
    trades_.insert(tradeId, trade);

    // 保存代码和报单号=
    trade->set_exchange(order->exchange());
    trade->set_symbol(order->symbol());
    trade->set_bforderid(order->bforderid());
    trade->set_tradeid(tradeId.toStdString());
    trade->set_direction(order->direction()); //方向=
    trade->set_offset(order->offset()); //开平=

    //价格、报单量等数值=
    trade->set_price(tick.lastprice());
    trade->set_volume(order->totalvolume());
    trade->set_tradedate(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString());
    trade->set_tradetime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());

    // ==pos & froze==
    QString longKey = QString().sprintf("%s-%s-long", trade->symbol().c_str(), trade->exchange().c_str());
    BfPositionData* longPos = positions_.value(longKey);
    int32_t newPos = longPos->position() + trade->volume();
    double newPrice = (longPos->position() * longPos->price() + trade->volume() * trade->price()) / newPos;
    longPos->set_price(newPrice);
    longPos->set_position(newPos);
    longPos->set_frozen(longPos->frozen() - trade->volume());

    // dispatch
    emit this->gotTrade(*trade);
    emit this->gotOrder(*order);
}

void GatewayMgr::onShortOpen(BfOrderData* order, const BfTickData& tick)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // ==order==
    order->set_tradedvolume(order->totalvolume());
    order->set_status(STATUS_ALLTRADED);

    // ==trade==
    BfTradeData* trade = new BfTradeData();
    QString tradeId = genTradeId();
    trades_.insert(tradeId, trade);

    // 保存代码和报单号=
    trade->set_exchange(order->exchange());
    trade->set_symbol(order->symbol());
    trade->set_bforderid(order->bforderid());
    trade->set_tradeid(tradeId.toStdString());
    trade->set_direction(order->direction()); //方向=
    trade->set_offset(order->offset()); //开平=

    //价格、报单量等数值=
    trade->set_price(tick.lastprice());
    trade->set_volume(order->totalvolume());
    trade->set_tradedate(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString());
    trade->set_tradetime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());

    // ==pos & frozen==
    QString shortKey = QString().sprintf("%s-%s-short", trade->symbol().c_str(), trade->exchange().c_str());
    BfPositionData* shortPos = positions_.value(shortKey);
    int32_t newPos = shortPos->position() + trade->volume();
    double newPrice = (shortPos->position() * shortPos->price() + trade->volume() * trade->price()) / newPos;
    shortPos->set_price(newPrice);
    shortPos->set_position(newPos);
    shortPos->set_frozen(shortPos->frozen() - trade->volume());

    // dispatch
    emit this->gotTrade(*trade);
    emit this->gotOrder(*order);
}

// 需要检查仓位是否足够平仓=
void GatewayMgr::onLongClose(BfOrderData* order, const BfTickData& tick)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // ==pos==
    QString shortKey = QString().sprintf("%s-%s-short", order->symbol().c_str(), order->exchange().c_str());
    BfPositionData* shortPos = positions_.value(shortKey);
    if (shortPos->position() < order->totalvolume()) {
        BfLog("no enough position for long+close");
        order->set_status(STATUS_CANCELLED); //状态=
        order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
        emit this->gotOrder(*order);
        return;
    }

    // ==order==
    order->set_tradedvolume(order->totalvolume());
    order->set_status(STATUS_ALLTRADED);

    // ==trade==
    BfTradeData* trade = new BfTradeData();
    QString tradeId = genTradeId();
    trades_.insert(tradeId, trade);

    // 保存代码和报单号=
    trade->set_exchange(order->exchange());
    trade->set_symbol(order->symbol());
    trade->set_bforderid(order->bforderid());
    trade->set_tradeid(tradeId.toStdString());
    trade->set_direction(order->direction()); //方向=
    trade->set_offset(order->offset()); //开平=

    //价格、报单量等数值=
    trade->set_price(tick.lastprice());
    trade->set_volume(order->totalvolume());
    trade->set_tradedate(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString());
    trade->set_tradetime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());

    //==pos==
    int32_t newPos = shortPos->position() - trade->volume();
    double newPrice = 0;
    if (newPos) {
        newPrice = (shortPos->position() * shortPos->price() - trade->volume() * trade->price()) / newPos;
    }
    shortPos->set_price(newPrice);
    shortPos->set_position(newPos);

    // dispatch
    emit this->gotTrade(*trade);
    emit this->gotOrder(*order);
}

// 需要检查仓位是否足够平仓=
void GatewayMgr::onShortClose(BfOrderData* order, const BfTickData& tick)
{
    BfLog(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // ==pos==
    QString longKey = QString().sprintf("%s-%s-long", order->symbol().c_str(), order->exchange().c_str());
    BfPositionData* longPos = positions_.value(longKey);
    if (longPos->position() < order->totalvolume()) {
        BfLog("no enough position for short+close");
        order->set_status(STATUS_CANCELLED); //状态=
        order->set_canceltime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());
        emit this->gotOrder(*order);
        return;
    }

    // ==order==
    order->set_tradedvolume(order->totalvolume());
    order->set_status(STATUS_ALLTRADED);

    // ==trade==
    BfTradeData* trade = new BfTradeData();
    QString tradeId = genTradeId();
    trades_.insert(tradeId, trade);

    // 保存代码和报单号=
    trade->set_exchange(order->exchange());
    trade->set_symbol(order->symbol());
    trade->set_bforderid(order->bforderid());
    trade->set_tradeid(tradeId.toStdString());
    trade->set_direction(order->direction()); //方向=
    trade->set_offset(order->offset()); //开平=

    //价格、报单量等数值=
    trade->set_price(tick.lastprice());
    trade->set_volume(order->totalvolume());
    trade->set_tradedate(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString());
    trade->set_tradetime(QDateTime::currentDateTime().toString("hh:mm:ss").toStdString());

    // ==pos==
    int32_t newPos = longPos->position() - trade->volume();
    double newPrice = 0;
    if (newPos) {
        newPrice = (longPos->position() * longPos->price() - trade->volume() * trade->price()) / newPos;
    }
    longPos->set_price(newPrice);
    longPos->set_position(newPos);

    // dispatch
    emit this->gotTrade(*trade);
    emit this->gotOrder(*order);
}
