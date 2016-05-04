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

    void onGotTick(QString gatewayId, const BfTickData& bfItem);
    void onGotTrade(QString gatewayId, const BfTradeData& bfItem);
    void onGotOrder(QString gatewayId, const BfOrderData& bfItem);

    /* stopped,idle,working*/
    /* 工作过程是这样的：停下robot，手动做一些工作比如撤单/平仓，然后可以重启*/
    /* working过程中，不要手动模式*，暂时由robot自己控制自己的生命周期*/
    /* 自动交易，cta自己也有个判断，不仅仅robot要被停止*/
    //void onInit();
    //void onFini();
    void onAutoTradingStart(); //cta打开了自动交易=
    void onAutoTradingStop(); //关闭了自动交易，回到idle状态=
private:
    QMap<QString, RobotClient*> clients_;
    QTimer* pingTimer_ = nullptr;
};

#endif // PUSHSERVICE_H
