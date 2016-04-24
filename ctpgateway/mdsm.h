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

// LOGIC
class MdSm : public QObject {
    Q_OBJECT
public:
    explicit MdSm(QObject* parent = 0);
    virtual ~MdSm();
    static QString version();

public:
    bool init(QString name, QString pwd, QString brokerId, QString front, QString flowPath);
    void start();
    void stop();

    void login(unsigned int delayTick);
    void subscrible(QStringList ids, unsigned int delayTick);
    void resetData();

signals:
    void statusChanged(int state);

private:
    QString userId_, password_, brokerId_, frontMd_, flowPathMd_;
    CThostFtdcMdApi* mdapi_ = nullptr;
    MdSmSpi* mdspi_ = nullptr;

    friend MdSmSpi;
};

#endif // MdSm_H
