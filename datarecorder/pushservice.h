#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QObject>
#include <QTimer>

#include "gatewaymgr.h"

class DatafeedClient;
// PUSH
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

public slots:
    void connectDatafeed(QString endpoint, QString clientId);
    void disconnectDatafeed();
    void onPing();

public slots:
    void onGotTick(QString gatewayId, const BfTickData& data);
    void onGotNotification(QString gatewayId, const BfNotificationData& data);

signals:

private:
    QTimer* pingTimer_ = nullptr;

    DatafeedClient* client_ = nullptr;
};

#endif // PUSHSERVICE_H
