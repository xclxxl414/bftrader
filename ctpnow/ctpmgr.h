#ifndef CTPMGR_H
#define CTPMGR_H

#include <QObject>

class MdSm;
class TdSm;
class Logger;
class Profile;

// 完成登录/自动登录/确认账单/订阅合约=
// 订阅什么合约由gateway统一确定，策略不管这事=
// 策略可以让gateway只转发部分合约的行情=
class CtpMgr : public QObject {
    Q_OBJECT
public:
    explicit CtpMgr(QObject* parent = 0);
    void init();
    void shutdown();

    TdSm* tdsm();
    MdSm* mdsm();
    bool running();

signals:
    void gotInstruments(QStringList ids);
    void gotTick(void* tick);

public slots:
    void showVersion();
    void start(QString password);
    void stop();

private slots:
    void onGotInstruments(QStringList ids);
    void onMdSmStateChanged(int state);
    void onTdSmStateChanged(int state);

private:
    Profile* profile();
    Logger* logger();
    bool initMdSm();
    void startMdSm();
    bool initTdSm();
    void startTdSm();
    void tryStartSubscrible();

private:
    MdSm* mdsm_ = nullptr;
    bool mdsm_logined_ = false;
    TdSm* tdsm_ = nullptr;
    bool tdsm_logined_ = false;
    bool autoLoginTd_ = true;
    bool autoLoginMd_ = true;

    QString password_;
};

#endif // CTPMGR_H
