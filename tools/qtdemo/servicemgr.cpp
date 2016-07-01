#include "servicemgr.h"
#include "ctamgr.h"
#include "dbservice.h"
#include "gatewaymgr.h"
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

    main_thread_ = QThread::currentThread();
    db_thread_ = new QThread;
    push_thread_ = new QThread;
    rpc_thread_ = new QThread;
    blogic_thread_ = new QThread;
    flogic_thread_ = new QThread;

    logger_ = new Logger;
    profile_ = new Profile;
    dbService_ = new DbService;
    dbService_->moveToThread(db_thread_);
    pushService_ = new PushService;
    pushService_->moveToThread(push_thread_);
    rpcService_ = new RpcService;
    rpcService_->moveToThread(rpc_thread_);
    gatewayMgr_ = new GatewayMgr;
    gatewayMgr_->moveToThread(blogic_thread_);
    ctaMgr_ = new CtaMgr;
    ctaMgr_->moveToThread(flogic_thread_);

    // ui objects
    logger_->init();
    profile_->init();

    QObject::connect(db_thread_, &QThread::started, this, &ServiceMgr::dbThreadStarted, Qt::DirectConnection);
    QObject::connect(push_thread_, &QThread::started, this, &ServiceMgr::pushThreadStarted, Qt::DirectConnection);
    QObject::connect(rpc_thread_, &QThread::started, this, &ServiceMgr::rpcThreadStarted, Qt::DirectConnection);
    QObject::connect(blogic_thread_, &QThread::started, this, &ServiceMgr::blogicThreadStarted, Qt::DirectConnection);
    QObject::connect(flogic_thread_, &QThread::started, this, &ServiceMgr::flogicThreadStarted, Qt::DirectConnection);
    QObject::connect(db_thread_, &QThread::finished, this, &ServiceMgr::dbThreadFinished, Qt::DirectConnection);
    QObject::connect(push_thread_, &QThread::finished, this, &ServiceMgr::pushThreadFinished, Qt::DirectConnection);
    QObject::connect(rpc_thread_, &QThread::finished, this, &ServiceMgr::rpcThreadFinished, Qt::DirectConnection);
    QObject::connect(blogic_thread_, &QThread::finished, this, &ServiceMgr::blogicThreadFinished, Qt::DirectConnection);
    QObject::connect(flogic_thread_, &QThread::finished, this, &ServiceMgr::flogicThreadFinished, Qt::DirectConnection);
    db_thread_->start();
    push_thread_->start();
    rpc_thread_->start();
    blogic_thread_->start();
    flogic_thread_->start();
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
    dbService_->moveToThread(main_thread_);
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
    pushService_->moveToThread(main_thread_);
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
    rpcService_->moveToThread(main_thread_);
}

void ServiceMgr::blogicThreadStarted()
{
    checkCurrentOn(BLOGIC);

    gatewayMgr_->init();
}

void ServiceMgr::blogicThreadFinished()
{
    checkCurrentOn(BLOGIC);

    gatewayMgr_->shutdown();
    gatewayMgr_->moveToThread(main_thread_);
}

void ServiceMgr::flogicThreadStarted()
{
    checkCurrentOn(FLOGIC);

    ctaMgr_->init();
}

void ServiceMgr::flogicThreadFinished()
{
    checkCurrentOn(FLOGIC);

    ctaMgr_->shutdown();
    ctaMgr_->moveToThread(main_thread_);
}

DbService* ServiceMgr::dbService()
{
    check();

    return this->dbService_;
}

PushService* ServiceMgr::pushService()
{
    check();

    return this->pushService_;
}

RpcService* ServiceMgr::rpcService()
{
    check();

    return this->rpcService_;
}

GatewayMgr* ServiceMgr::gatewayMgr()
{
    check();

    return this->gatewayMgr_;
}

CtaMgr* ServiceMgr::ctaMgr()
{
    check();

    return this->ctaMgr_;
}

void ServiceMgr::shutdown()
{
    if (shutdown_ == true) {
        qFatal("shutdown_ == true");
        return;
    }

    flogic_thread_->quit();
    flogic_thread_->wait();
    delete flogic_thread_;
    flogic_thread_ = nullptr;

    blogic_thread_->quit();
    blogic_thread_->wait();
    delete blogic_thread_;
    blogic_thread_ = nullptr;

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

    profile_->shutdown();
    logger_->shutdown();

    delete ctaMgr_;
    ctaMgr_ = nullptr;

    delete gatewayMgr_;
    gatewayMgr_ = nullptr;

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

    main_thread_ = nullptr;

    QThreadPool::globalInstance()->waitForDone();

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

    if (p == ServiceMgr::MAIN) {
        return this->main_thread_;
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
    if (p == ServiceMgr::BLOGIC) {
        return this->blogic_thread_;
    }
    if (p == ServiceMgr::FLOGIC) {
        return this->flogic_thread_;
    }

    qFatal("getThread");
    return nullptr;
}

bool ServiceMgr::isCurrentOn(ServiceMgr::ThreadType p)
{
    check();

    QThread* cur = QThread::currentThread();
    if (p == ServiceMgr::MAIN && cur == main_thread_) {
        return true;
    }

    if (p == ServiceMgr::DB && cur == db_thread_) {
        return true;
    }

    if (p == ServiceMgr::PUSH && cur == push_thread_) {
        return true;
    }

    if (p == ServiceMgr::RPC && cur == rpc_thread_) {
        return true;
    }

    if (p == ServiceMgr::BLOGIC && cur == blogic_thread_) {
        return true;
    }

    if (p == ServiceMgr::FLOGIC && cur == flogic_thread_) {
        return true;
    }

    if (p == ServiceMgr::EXTERNAL) {
        if (cur != main_thread_ && cur != db_thread_
            && cur != push_thread_ && cur != rpc_thread_
            && cur != blogic_thread_ && cur != flogic_thread_) {
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

void BfLog(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);

    g_sm->logger()->log(buf);
}

void BfLog(QString msg)
{
    g_sm->logger()->log(msg);
}
