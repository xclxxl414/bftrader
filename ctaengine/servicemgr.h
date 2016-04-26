#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>

class QThread;
class Logger;
class Profile;
class CtpMgr;
class DbService;
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
        EXTERNAL, // threadpool
        UI, // ui
        DB, // database
        IO, // file
        PUSH, // network-->
        RPC, // network<--
        LOGIC // logic,eg.ctpmgr
    };
    QThread* getThread(ThreadType p);
    bool isCurrentOn(ThreadType p);
    void checkCurrentOn(ThreadType p);

    Logger* logger();
    Profile* profile();
    CtpMgr* ctpMgr();
    DbService* dbService();
    RpcService* rpcService();
    PushService* pushService();

private slots:
    void dbThreadStarted();
    void ioThreadStarted();
    void pushThreadStarted();
    void rpcThreadStarted();
    void logicThreadStarted();

    void dbThreadFinished();
    void ioThreadFinished();
    void pushThreadFinished();
    void rpcThreadFinished();
    void logicThreadFinished();

private:
    void check();

private:
    QThread* ui_thread_ = nullptr;
    QThread* db_thread_ = nullptr;
    QThread* io_thread_ = nullptr;
    QThread* push_thread_ = nullptr;
    QThread* rpc_thread_ = nullptr;
    QThread* logic_thread_ = nullptr;

    Logger* logger_ = nullptr;
    Profile* profile_ = nullptr;
    CtpMgr* ctpMgr_ = nullptr;
    DbService* dbService_ = nullptr;
    RpcService* rpcService_ = nullptr;
    PushService* pushService_ = nullptr;

    bool shutdown_ = false;
    bool init_ = false;
};

extern ServiceMgr* g_sm;

void BfError(const char* msg, ...);
void BfInfo(const char* msg, ...);
void BfDebug(const char* msg, ...);

void BfError(QString msg);
void BfInfo(QString msg);
void BfDebug(QString msg);

#endif // SERVICEMGR_H
