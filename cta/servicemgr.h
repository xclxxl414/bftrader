#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>

class QThread;
class Logger;
class Profile;
class DbService;
class PushService;
class RpcService;
class GatewayMgr;
class CtaMgr;

class ServiceMgr : public QObject {
    Q_OBJECT
public:
    explicit ServiceMgr(QObject* parent = 0);
    void init();
    void shutdown();

public:
    enum ThreadType {
        EXTERNAL, // threadpool
        MAIN, // main
        DB, // database
        PUSH, // network-->
        RPC, // network<--
        BLOGIC, // backend logic,eg.gatewaymgr
        FLOGIC, // frontend logic,eg.ctamgr
    };
    QThread* getThread(ThreadType p);
    bool isCurrentOn(ThreadType p);
    void checkCurrentOn(ThreadType p);

    Logger* logger();
    Profile* profile();
    DbService* dbService();
    PushService* pushService();
    RpcService* rpcService();
    GatewayMgr* gatewayMgr();
    CtaMgr* ctaMgr();

private slots:
    void dbThreadStarted();
    void pushThreadStarted();
    void rpcThreadStarted();
    void blogicThreadStarted();
    void flogicThreadStarted();

    void dbThreadFinished();
    void pushThreadFinished();
    void rpcThreadFinished();
    void blogicThreadFinished();
    void flogicThreadFinished();

private:
    void check();

private:
    QThread* main_thread_ = nullptr;
    QThread* db_thread_ = nullptr;
    QThread* push_thread_ = nullptr;
    QThread* rpc_thread_ = nullptr;
    QThread* blogic_thread_ = nullptr;
    QThread* flogic_thread_ = nullptr;

    Logger* logger_ = nullptr;
    Profile* profile_ = nullptr;
    DbService* dbService_ = nullptr;
    PushService* pushService_ = nullptr;
    RpcService* rpcService_ = nullptr;
    GatewayMgr* gatewayMgr_ = nullptr;
    CtaMgr* ctaMgr_ = nullptr;

    bool shutdown_ = false;
    bool init_ = false;
};

extern ServiceMgr* g_sm;

void BfLog(const char* msg, ...);
void BfLog(QString msg);

#endif // SERVICEMGR_H
