#pragma once

#include <QObject>
#include <QTimer>

#include "gatewaymgr.h"

class DatafeedClient;

//
// daservice负责行情：提供tick的回放，tick从datafeed读取
// gatewayMgr负责交易，也是行情的消费者
//

// DB
class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();

    //线程安全=
    bool getContract(const BfGetContractReq& req, QList<BfContractData>& resp);

public slots:
    void connectDatafeed(QString endpoint, QString clientId);
    void disconnectDatafeed();
    void onPing();

    void onTradeStopped();
    void onTradeWillBegin(const BfGetTickReq& req);

signals:
    void gotContracts();
    void gotTick(const BfTickData& tick);

private:
    QTimer* pingTimer_ = nullptr;

    DatafeedClient* client_ = nullptr;
};
