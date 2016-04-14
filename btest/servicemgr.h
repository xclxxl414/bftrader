#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>

class QThread;
class Logger;
class Profile;
class CtpMgr;
class LeveldbBackend;
class RpcService;
class PushService;

class ServiceMgr : public QObject {
    Q_OBJECT
public:
    explicit ServiceMgr(QObject* parent = 0);
    void init();
    void shutdown();

public:
    enum ThreadType {
        EXTERNAL,
        UI,
        DB,
        IO,
        CTP
    };
    QThread* getThread(ThreadType p);
    bool isCurrentOn(ThreadType p);
    void checkCurrentOn(ThreadType p);

    Logger* logger();
    Profile* profile();
    CtpMgr* ctpMgr();
    LeveldbBackend* leveldbBackend();
    RpcService* rpcService();
    PushService* pushService();

private slots:
    void dbThreadStarted();
    void ioThreadStarted();
    void ctpThreadStarted();
    void dbThreadFinished();
    void ioThreadFinished();
    void ctpThreadFinished();

private:
    void check();

private:
    QThread* ui_thread_ = nullptr;
    QThread* db_thread_ = nullptr;
    QThread* io_thread_ = nullptr;
    QThread* ctp_thread_ = nullptr;

    Logger* logger_ = nullptr;
    Profile* profile_ = nullptr;
    CtpMgr* ctpMgr_ = nullptr;
    LeveldbBackend* leveldbBackend_ = nullptr;
    RpcService* rpcService_ = nullptr;
    PushService* pushService_ = nullptr;

    bool shutdown_ = false;
    bool init_ = false;
};

extern ServiceMgr* g_sm;

#endif // SERVICEMGR_H
