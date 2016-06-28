#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include "bfdatafeed.pb.h"
#include "bfgateway.pb.h"

#include <QMap>
#include <QObject>
#include <atomic>

using namespace bfgateway;
using namespace bfdatafeed;

// 每次回测都初始化一些数据，里面放
// 1.account信息
// 2.contract信息
// 3.每笔交易
// 4.每笔盈亏
// 5.回测参数
// ....
// 回测报告根据这些数据来算=
// 盈亏曲线计算方法:（Beta1）
// 1. 按成交信息，记录所有的开仓，现金-=开仓数量*合约乘数*价格
// 2. 按成交信息，记录所有的平仓，现金+=平仓数量*合约乘数*价格
//
// 盈亏分布计算方法：依赖于每笔盈亏（beta2）
// 1. 按成交信息，记录所有的开仓，记录总pos 和 平均price
// 2. 按成交信息，记录所有的平仓，如多：盈亏 = 平仓数量*（当前价格-持仓价格）*合约乘数
// 3. 找出最大盈利 和 最大亏损，做出分布区间，平均为20个子区间来求分布

// 价格撮合：
// 1. 委托价格被触发后，以委托价格+-滑点来作为成交价格
// 2. 平仓 开仓都要计算一次滑点
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
    void sendOrderWithId(QString byOrderId, const BfSendOrderReq& req);
    void sendOrder(const BfSendOrderReq& req);
    void queryPosition();
    void cancelOrder(const BfCancelOrderReq& req);
    void queryOrders();

    void onGotTick(const BfTickData& tick);

private:
    void resetData();
    int getOrderId();

private:
    std::atomic_int32_t orderId_ = 0;
    int tradeId_ = 0;
    BfAccountData account_;
    QMap<QString, BfPositionData*> positions_;
    QMap<QString, BfOrderData*> orders_;
    QMap<QString, BfTradeData*> trades_;
};

#endif // GATEWAYMGR_H
