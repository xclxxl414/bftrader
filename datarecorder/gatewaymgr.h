#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include <QMap>
#include <QMutex>
#include <QObject>

class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();

signals:

private:
};

#endif // GATEWAYMGR_H
