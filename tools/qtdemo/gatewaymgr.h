#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include <QObject>

//BLOGC
class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void showVersion();
};

#endif // GATEWAYMGR_H
