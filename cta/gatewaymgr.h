#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include "bftrader.pb.h"
#include <QMap>
#include <QObject>
#include <QTimer>

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

public slots:
    void startProxy();
    void stopProxy();
    void connectGateway(QString gatewayId, QString endpoint, const BfConnectReq& req);
    void disconnectGateway(QString gatewayId);
    void onProxyClosed();
    void onPing();

    // todo(hege)
    void getContract(const BfGetContractReq& req, BfContractData& resp);
    void sendOrder(const BfSendOrderReq& req, BfSendOrderResp& resp);
    void cancelOrder(const BfCancelOrderReq& req, BfVoid& resp);
    void queryAccount(const BfVoid& req, BfVoid& resp);
    void queryPosition(const BfVoid& req, BfVoid& resp);
    void queryOrders(const BfVoid& req, BfVoid& resp);

signals:
    void tradeWillBegin(const BfVoid& data);
    void gotContracts(const BfVoid& data);
    void gotTick(const BfTickData& data);
    void gotPing(const BfPingData& data);
    void gotAccount(const BfAccountData& data);
    void gotOrder(const BfOrderData& data);
    void gotTrade(const BfTradeData& data);
    void gotPosition(const BfPositionData& data);
    void gotError(const BfErrorData& data);
    void gotLog(const BfLogData& data);

private slots:
    void onProxyThreadStarted();

private:
    QThread* proxyThread_ = nullptr;
    grpc::Server* grpcServer_ = nullptr;

    QMap<QString, GatewayClient*> clients_;
    QTimer* pingTimer_ = nullptr;
};

#endif // GATEWAYMGR_H
