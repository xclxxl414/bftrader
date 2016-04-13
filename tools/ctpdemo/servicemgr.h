#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>

class QThread;
class Logger;
class Profile;
class CtpCmdMgr;
class CtpMgr;
class DataPump;
class DbService;

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
    CtpCmdMgr* ctpCmdMgr();
    CtpMgr* ctpMgr();
    DataPump* dataPump();
    DbService* dbService();

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
    CtpCmdMgr* ctpCmdMgr_ = nullptr;
    CtpMgr* ctpMgr_ = nullptr;
    DataPump* dataPump_ = nullptr;
    DbService* dbservice_ = nullptr;

    bool shutdown_ = false;
    bool init_ = false;
};

extern ServiceMgr* g_sm;

#endif // SERVICEMGR_H
