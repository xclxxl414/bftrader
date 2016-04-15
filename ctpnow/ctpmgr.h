#ifndef CTPMGR_H
#define CTPMGR_H

#include <QObject>

class MdSm;
class TdSm;
class Logger;
class Profile;

//
// 1. 完成登录/自动登录/确认账单/查询可交易合约列表=
// 2. 实现自动订阅(因为策略连gateway时候，gateway并不一定连接了柜台)=
// 3. 非常策略主动取消订阅，合约的订阅会一直自动订阅下去=
//
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
