#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include "bfgateway.pb.h"
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QTimer>

using namespace bfgateway;

class GatewayClient;

//
// 通过gatewaymgr来桥接ctpgateway
//
class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();

    //线程安全=
    void getContract(QString gatewayId, const BfGetContractReq& req, QList<BfContractData>& resp);
    void sendOrder(QString gatewayId, const BfSendOrderReq& req, BfSendOrderResp& resp);

public slots:
    void connectGateway(QString gatewayId, QString endpoint, const BfConnectPushReq& req);
    void disconnectGateway(QString gatewayId);
    void onGatewayDisconnected(QString gatewayId);
    void onPing();

    void cancelOrder(QString gatewayId, const BfCancelOrderReq& req);
    void queryAccount(QString gatewayId);
    void queryPosition(QString gatewayId);
    void queryOrders(QString gatewayId);

signals:
    void gotPing(QString gatewayId, const BfPingData& data);
    void gotTick(QString gatewayId, const BfTickData& data);
    void gotAccount(QString gatewayId, const BfAccountData& data);
    void gotOrder(QString gatewayId, const BfOrderData& data);
    void gotTrade(QString gatewayId, const BfTradeData& data);
    void gotPosition(QString gatewayId, const BfPositionData& data);
    void gotError(QString gatewayId, const BfErrorData& data);
    void gotLog(QString gatewayId, const BfLogData& data);
    void gotNotification(QString gatewayId, const BfNotificationData& data);

private:
    QMap<QString, GatewayClient*> clients_;
    QMutex clients_mutex_;
    QTimer* pingTimer_ = nullptr;
};

#endif // GATEWAYMGR_H
