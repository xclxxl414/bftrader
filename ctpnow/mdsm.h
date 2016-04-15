#ifndef MdSm_H
#define MdSm_H

#include <QObject>

class CThostFtdcMdApi;
class MdSmSpi;

enum {
    MDSM_DISCONNECTED = 1,
    MDSM_CONNECTED,
    MDSM_LOGINED,
    MDSM_LOGINFAIL,
    MDSM_STOPPED
};

//CTP
class MdSm : public QObject {
    Q_OBJECT
public:
    explicit MdSm(QObject* parent = 0);
    virtual ~MdSm();

public:
    bool init(QString name, QString pwd, QString brokerId, QString front, QString flowPath);
    void start();
    void stop();
    void info(QString msg);

    void login(unsigned int delayTick,QString robotId);
    void subscrible(QStringList ids,unsigned int delayTick,QString robotId);

signals:
    void statusChanged(int state);
    void requestSent(int reqId,QString robotId);

private:
    QString userId_,password_,brokerId_,frontMd_,flowPathMd_;
    CThostFtdcMdApi* mdapi_ = nullptr;
    MdSmSpi* mdspi_ = nullptr;
    int reqId_ = 0;
    int RESEND_AFTER_MSEC = 1000;

    friend MdSmSpi;
};

#endif // MdSm_H
