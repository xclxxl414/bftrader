#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include "gatewaymgr.h"
#include <QMap>
#include <QObject>
#include <QTimer>

class CtaClient;

// PUSH
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void connectClient(QString ctaId, const BfConnectPushReq& req, void* queue);
    void disconnectClient(QString clientId);
    void onCtaClosed();
    void onPing();

    void onGotTick(QString gatewayId, const BfTickData& bfItem);
    void onGotTrade(QString gatewayId, const BfTradeData& bfItem);
    void onGotOrder(QString gatewayId, const BfOrderData& bfItem);

private:
    QMap<QString, CtaClient*> clients_;
    QTimer* pingTimer_ = nullptr;
};

#endif // PUSHSERVICE_H
