#include "servicemgr.h"
#include "ctpmgr.h"
#include "dbservice.h"
#include "logger.h"
#include "profile.h"
#include "pushservice.h"
#include "rpcservice.h"
#include <QThread>
#include <QThreadPool>

ServiceMgr* g_sm = nullptr;

ServiceMgr::ServiceMgr(QObject* parent)
    : QObject(parent)
{
    g_sm = this;
}

void ServiceMgr::init()
{
    if (init_ == true) {
        qFatal("init_ == true");
        return;
    }
    init_ = true;

    const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(qMax(4, 2 * threadCount));

    ui_thread_ = QThread::currentThread();
    io_thread_ = new QThread;
    db_thread_ = new QThread;
    push_thread_ = new QThread;
    rpc_thread_ = new QThread;
    logic_thread_ = new QThread;

    logger_ = new Logger;
    profile_ = new Profile;
    dbService_ = new DbService;
    dbService_->moveToThread(db_thread_);
    rpcService_ = new RpcService;
    rpcService_->moveToThread(rpc_thread_);
    pushService_ = new PushService;
    pushService_->moveToThread(push_thread_);
    ctpMgr_ = new CtpMgr;
    ctpMgr_->moveToThread(logic_thread_);

    // ui objects
    logger_->init();
    profile_->init();

    QObject::connect(io_thread_, &QThread::started, this, &ServiceMgr::ioThreadStarted, Qt::DirectConnection);
    QObject::connect(db_thread_, &QThread::started, this, &ServiceMgr::dbThreadStarted, Qt::DirectConnection);
    QObject::connect(push_thread_, &QThread::started, this, &ServiceMgr::pushThreadStarted, Qt::DirectConnection);
    QObject::connect(rpc_thread_, &QThread::started, this, &ServiceMgr::rpcThreadStarted, Qt::DirectConnection);
    QObject::connect(logic_thread_, &QThread::started, this, &ServiceMgr::logicThreadStarted, Qt::DirectConnection);
    QObject::connect(io_thread_, &QThread::finished, this, &ServiceMgr::ioThreadFinished, Qt::DirectConnection);
    QObject::connect(db_thread_, &QThread::finished, this, &ServiceMgr::dbThreadFinished, Qt::DirectConnection);
    QObject::connect(push_thread_, &QThread::finished, this, &ServiceMgr::pushThreadFinished, Qt::DirectConnection);
    QObject::connect(rpc_thread_, &QThread::finished, this, &ServiceMgr::rpcThreadFinished, Qt::DirectConnection);
    QObject::connect(logic_thread_, &QThread::finished, this, &ServiceMgr::logicThreadFinished, Qt::DirectConnection);
    io_thread_->start();
    db_thread_->start();
    push_thread_->start();
    rpc_thread_->start();
    logic_thread_->start();
}

void ServiceMgr::ioThreadStarted()
{
    checkCurrentOn(IO);
}

void ServiceMgr::ioThreadFinished()
{
    checkCurrentOn(IO);
}

void ServiceMgr::dbThreadStarted()
{
    checkCurrentOn(DB);

    dbService_->init();
}

void ServiceMgr::dbThreadFinished()
{
    checkCurrentOn(DB);

    dbService_->shutdown();
    dbService_->moveToThread(ui_thread_);
}

void ServiceMgr::pushThreadStarted()
{
    checkCurrentOn(PUSH);

    pushService_->init();
}

void ServiceMgr::pushThreadFinished()
{
    checkCurrentOn(PUSH);

    pushService_->shutdown();
    pushService_->moveToThread(ui_thread_);
}

void ServiceMgr::rpcThreadStarted()
{
    checkCurrentOn(RPC);

    rpcService_->init();
}

void ServiceMgr::rpcThreadFinished()
{
    checkCurrentOn(RPC);

    rpcService_->shutdown();
    rpcService_->moveToThread(ui_thread_);
}

void ServiceMgr::logicThreadStarted()
{
    checkCurrentOn(LOGIC);

    ctpMgr_->init();
}

void ServiceMgr::logicThreadFinished()
{
    checkCurrentOn(LOGIC);

    ctpMgr_->shutdown();
    ctpMgr_->moveToThread(ui_thread_);
}

CtpMgr* ServiceMgr::ctpMgr()
{
    check();

    return this->ctpMgr_;
}

DbService* ServiceMgr::dbService()
{
    check();

    return this->dbService_;
}

RpcService* ServiceMgr::rpcService()
{
    check();

    return this->rpcService_;
}

PushService* ServiceMgr::pushService()
{
    check();

    return this->pushService_;
}

void ServiceMgr::shutdown()
{
    if (shutdown_ == true) {
        qFatal("shutdown_ == true");
        return;
    }

    QThreadPool::globalInstance()->waitForDone();

    logic_thread_->quit();
    logic_thread_->wait();
    delete logic_thread_;
    logic_thread_ = nullptr;

    rpc_thread_->quit();
    rpc_thread_->wait();
    delete rpc_thread_;
    rpc_thread_ = nullptr;

    push_thread_->quit();
    push_thread_->wait();
    delete push_thread_;
    push_thread_ = nullptr;

    db_thread_->quit();
    db_thread_->wait();
    delete db_thread_;
    db_thread_ = nullptr;

    io_thread_->quit();
    io_thread_->wait();
    delete io_thread_;
    io_thread_ = nullptr;

    profile_->shutdown();
    logger_->shutdown();

    delete ctpMgr_;
    ctpMgr_ = nullptr;

    delete rpcService_;
    rpcService_ = nullptr;

    delete pushService_;
    pushService_ = nullptr;

    delete dbService_;
    dbService_ = nullptr;

    delete profile_;
    profile_ = nullptr;
    delete logger_;
    logger_ = nullptr;

    ui_thread_ = nullptr;

    shutdown_ = true;
}

void ServiceMgr::check()
{
    if (shutdown_ || !init_) {
        qFatal("shutdown_ || !init_");
    }
}

Profile* ServiceMgr::profile()
{
    check();

    return this->profile_;
}

Logger* ServiceMgr::logger()
{
    check();

    return this->logger_;
}

QThread* ServiceMgr::getThread(ThreadType p)
{
    check();

    if (p == ServiceMgr::UI) {
        return this->ui_thread_;
    }
    if (p == ServiceMgr::IO) {
        return this->io_thread_;
    }
    if (p == ServiceMgr::DB) {
        return this->db_thread_;
    }
    if (p == ServiceMgr::PUSH) {
        return this->push_thread_;
    }
    if (p == ServiceMgr::RPC) {
        return this->rpc_thread_;
    }
    if (p == ServiceMgr::LOGIC) {
        return this->logic_thread_;
    }

    qFatal("getThread");
    return nullptr;
}

bool ServiceMgr::isCurrentOn(ServiceMgr::ThreadType p)
{
    check();

    QThread* cur = QThread::currentThread();
    if (p == ServiceMgr::UI && cur == ui_thread_) {
        return true;
    }

    if (p == ServiceMgr::DB && cur == db_thread_) {
        return true;
    }

    if (p == ServiceMgr::IO && cur == io_thread_) {
        return true;
    }

    if (p == ServiceMgr::PUSH && cur == push_thread_) {
        return true;
    }

    if (p == ServiceMgr::RPC && cur == rpc_thread_) {
        return true;
    }

    if (p == ServiceMgr::LOGIC && cur == logic_thread_) {
        return true;
    }

    if (p == ServiceMgr::EXTERNAL) {
        if (cur != ui_thread_ && cur != db_thread_
            && cur != io_thread_ && cur != push_thread_
            && cur != rpc_thread_ && cur != logic_thread_) {
            return true;
        }
    }

    return false;
}

void ServiceMgr::checkCurrentOn(ThreadType p)
{
    if (!isCurrentOn(p)) {
        qFatal("checkCurrentOn");
    }
}

//////////////////////

void BfError(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);

    g_sm->logger()->error(buf);
}

void BfInfo(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);

    g_sm->logger()->info(buf);
}

void BfDebug(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);

    g_sm->logger()->debug(buf);
}

void BfError(QString msg)
{
    g_sm->logger()->error(msg);
}

void BfInfo(QString msg)
{
    g_sm->logger()->info(msg);
}

void BfDebug(QString msg)
{
    g_sm->logger()->info(msg);
}
