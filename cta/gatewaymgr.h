#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include "bftrader.pb.h"
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QMutex>

using namespace bftrader;

class GatewayClient;
namespace grpc {
class Server;
}

//
// 通过gatewaymgr来桥接ctpgateway
// 后期重构，将ctpgateway改成dll，直接加载应该方便一些=
//

class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();

    // ui
public slots:
    void startProxy();
    void stopProxy();
    void connectGateway(QString gatewayId, QString endpoint, const BfConnectReq& req);
    void disconnectGateway(QString gatewayId);
    void onProxyClosed();
    void onPing();

    // channel&stub is threadsafe
public slots:
    void getContract(QString gatewayId,const BfGetContractReq& req, BfContractData& resp);
    void sendOrder(QString gatewayId,const BfSendOrderReq& req, BfSendOrderResp& resp);
    void cancelOrder(QString gatewayId,const BfCancelOrderReq& req);
    void queryAccount(QString gatewayId);
    void queryPosition(QString gatewayId);

signals:
    void tradeWillBegin(QString gatewayId);
    void gotContracts(QString gatewayId);
    void gotPing(QString gatewayId,const BfPingData& data);
    void gotTick(QString gatewayId,const BfTickData& data);
    void gotAccount(QString gatewayId,const BfAccountData& data);
    void gotOrder(QString gatewayId,const BfOrderData& data);
    void gotTrade(QString gatewayId,const BfTradeData& data);
    void gotPosition(QString gatewayId,const BfPositionData& data);
    void gotError(QString gatewayId,const BfErrorData& data);
    void gotLog(QString gatewayId,const BfLogData& data);

private slots:
    void onProxyThreadStarted();

private:
    QThread* proxyThread_ = nullptr;
    grpc::Server* grpcServer_ = nullptr;

    QMap<QString, GatewayClient*> clients_;
    QMutex clients_mutex_;
    QTimer* pingTimer_ = nullptr;
};

#endif // GATEWAYMGR_H
