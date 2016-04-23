#ifndef TdSm_H
#define TdSm_H

#include "bftrader.pb.h"
#include <QObject>

using namespace bftrader;

class CThostFtdcTraderApi;

enum {
    TDSM_DISCONNECTED = 1,
    TDSM_CONNECTED,
    TDSM_LOGINED,
    TDSM_LOGINFAIL,
    TDSM_LOGOUTED,
    TDSM_LOGOUTFAIL,
    TDSM_STOPPED
};

class TdSmSpi;

// LOGIC
class TdSm : public QObject {
    Q_OBJECT
public:
    explicit TdSm(QObject* parent = 0);
    virtual ~TdSm();
    static QString version();

public:
    bool init(QString userId, QString password, QString brokerId, QString frontTd, QString flowPathTd, QString idPrefixList);
    void start();
    void stop();
    void resetData();
    QString genBfOrderId();

    void login(unsigned int delayTick);
    void logout(unsigned int delayTick);
    void queryInstrument(unsigned int delayTick);
    void queryAccount(unsigned int delayTick);
    void reqSettlementInfoConfirm(unsigned int delayTick);
    void sendOrder(unsigned int delayTick, const BfSendOrderReq& orderReq);
    void sendOrder(unsigned int delayTick, QString bfOrderId, const BfSendOrderReq& orderReq);
    void queryPosition(unsigned int delayTick);
    void cancelOrder(unsigned int delayTick, const BfCancelOrderReq& orderReq);
    void queryOrders(unsigned int delayTick);

signals:
    void statusChanged(int state);

private:
    QString userId_, password_, brokerId_, frontTd_, flowPathTd_, idPrefixList_;
    CThostFtdcTraderApi* tdapi_ = nullptr;
    TdSmSpi* tdspi_ = nullptr;

    friend TdSmSpi;
};

#endif // TdSm_H
