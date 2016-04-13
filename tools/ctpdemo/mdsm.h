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
    static QString version();
    bool init(QString name, QString pwd, QString brokerId, QString front, QString flowPath);
    void start();
    void login(unsigned int delayTick);
    void stop();
    void subscrible(QStringList ids);

signals:
    void statusChanged(int state);
    void runCmd(void* cmd,unsigned int delayTick);

protected:
    CThostFtdcMdApi* mdapi() { return mdapi_; }
    QString brokerId() { return brokerId_; }
    QString userId() { return userId_; }
    QString password() { return password_; }
    void info(QString msg);

private:
    QString userId_,password_,brokerId_,frontMd_,flowPathMd_;
    CThostFtdcMdApi* mdapi_ = nullptr;
    MdSmSpi* mdspi_ = nullptr;

    friend MdSmSpi;
};

#endif // MdSm_H
