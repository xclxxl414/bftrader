#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include "bfdatafeed.pb.h"
#include "bfgateway.pb.h"

#include <QMap>
#include <QObject>
#include <atomic>

using namespace bfgateway;
using namespace bfdatafeed;

//
// 只支持一个品种
// 只支持平今
// 只支持限价单
// 只支持1-500手/每单
// 资金无限制,除account信息无效外，其他都有效=
//
class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();

    // 分配BfOrderId
    QString genOrderId();

signals:
    void tradeWillBegin(const BfGetTickReq& req);
    void tradeStopped();
    void gotAccount(const BfAccountData& account);
    void gotOrder(const BfOrderData& order);
    void gotTrade(const BfTradeData& trade);
    void gotPosition(const BfPositionData& pos);
    void gotGatewayError(int code, QString msg, QString msgEx);
    void gotNotification(const BfNotificationData& note);

public slots:
    void start(const BfGetTickReq& req);
    void stop();
    void queryAccount();
    void sendOrderWithId(QString bfOrderId, const BfSendOrderReq& req);
    void sendOrder(const BfSendOrderReq& req);
    void queryPosition();
    void cancelOrder(const BfCancelOrderReq& req);
    void queryOrders();

    void onGotTick(const BfTickData& tick);

private:
    void resetData();
    int getOrderId();
    int getTradeId();
    QString genTradeId();

    void onLongOpen(BfOrderData* order, const BfTickData& tick);
    void onShortOpen(BfOrderData* order, const BfTickData& tick);
    void onLongClose(BfOrderData* order, const BfTickData& tick);
    void onShortClose(BfOrderData* order, const BfTickData& tick);

private:
    std::atomic_int32_t orderId_ = 0;
    int tradeId_ = 0;
    BfAccountData account_;
    QMap<QString, BfPositionData*> positions_;
    QMap<QString, BfOrderData*> orders_;
    QMap<QString, BfTradeData*> trades_;
};

#endif // GATEWAYMGR_H
