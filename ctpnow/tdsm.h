#ifndef TdSm_H
#define TdSm_H

#include <QObject>

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

//CTP
class TdSm : public QObject {
    Q_OBJECT
public:
    explicit TdSm(QObject* parent = 0);
    virtual ~TdSm();

public:
    bool init(QString userId, QString password, QString brokerId, QString frontTd, QString flowPathTd, QString idPrefixList);
    void start();
    void stop();
    void info(QString msg);

    void login(unsigned int delayTick, QString robotId);
    void logout(unsigned int delayTick, QString robotId);
    void queryInstrument(unsigned int delayTick, QString robotId);

signals:
    void statusChanged(int state);
    void gotInstruments(QStringList ids);
    void requestSent(int reqId, QString robotId);

private:
    QString userId_, password_, brokerId_, frontTd_, flowPathTd_, idPrefixList_;
    CThostFtdcTraderApi* tdapi_ = nullptr;
    TdSmSpi* tdspi_ = nullptr;
    int reqId_ = 1;
    int RESEND_AFTER_MSEC = 1000;

    friend TdSmSpi;
};

#endif // TdSm_H
