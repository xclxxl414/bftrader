#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include <QObject>

#include "bftrader.pb.h"
using namespace bftrader;

class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();
};

#endif // GATEWAYMGR_H
