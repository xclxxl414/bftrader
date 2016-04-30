#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include <QObject>

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

signals:
};

#endif // GATEWAYMGR_H
