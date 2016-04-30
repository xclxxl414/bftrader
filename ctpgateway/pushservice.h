#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QMap>
#include <QObject>
#include <QTimer>

#include "gatewaymgr.h"

class ProxyClient;

// PUSH
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void onClientConnect(const BfConnectReq& req);
    void onClientClose(QString clientId);
    void onServerClose();

    void onTradeWillBegin();
    void onGotContracts(QStringList ids, QStringList idsAll);
    void onPing();
    void onCtpError(int code, QString msg, QString msgEx);
    void onLog(QString when, QString msg);
    void onGotTick(void* curTick, void* preTick);
    void onGotTrade(const BfTradeData& trade);
    void onGotOrder(const BfOrderData& data);
    void onGotPosition(const BfPositionData& pos);
    void onGotAccount(const BfAccountData& account);

private:
    QMap<QString, ProxyClient*> clients_;
    QTimer* pingTimer_ = nullptr;
};

#endif // PUSHSERVICE_H
