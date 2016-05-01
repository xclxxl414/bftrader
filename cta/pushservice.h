#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include "gatewaymgr.h"
#include <QMap>
#include <QObject>
#include <QTimer>

class RobotClient;

// PUSH
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void connectRobot(QString ctaId, const BfConnectReq& req);
    void disconnectRobot(QString robotId);
    void onCtaClosed();
    void onPing();

private:
    QMap<QString, RobotClient*> clients_;
    QTimer* pingTimer_ = nullptr;
};

#endif // PUSHSERVICE_H
