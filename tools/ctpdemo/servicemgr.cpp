#include "servicemgr.h"
#include "profile.h"
#include "logger.h"
#include "ctpcmdmgr.h"
#include "ctpmgr.h"
#include "datapump.h"
#include "dbservice.h"
#include <QThread>

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

    ui_thread_ = QThread::currentThread();
    io_thread_ = new QThread;
    db_thread_ = new QThread;
    ctp_thread_ = new QThread;

    logger_ = new Logger;
    profile_ = new Profile;

    ctpCmdMgr_ = new CtpCmdMgr;
    ctpCmdMgr_->moveToThread(ctp_thread_);
    ctpMgr_ = new CtpMgr;
    ctpMgr_->moveToThread(ctp_thread_);
    dataPump_ = new DataPump;
    dataPump_->moveToThread(ctp_thread_);

    dbservice_ = new DbService;
    dbservice_->moveToThread(db_thread_);

    logger_->init();
    profile_->init();

    QObject::connect(io_thread_, &QThread::started, this, &ServiceMgr::ioThreadStarted, Qt::DirectConnection);
    QObject::connect(ctp_thread_, &QThread::started, this, &ServiceMgr::ctpThreadStarted, Qt::DirectConnection);
    QObject::connect(db_thread_, &QThread::started, this, &ServiceMgr::dbThreadStarted, Qt::DirectConnection);
    QObject::connect(io_thread_, &QThread::finished, this, &ServiceMgr::ioThreadFinished, Qt::DirectConnection);
    QObject::connect(ctp_thread_, &QThread::finished, this, &ServiceMgr::ctpThreadFinished, Qt::DirectConnection);
    QObject::connect(db_thread_, &QThread::finished, this, &ServiceMgr::dbThreadFinished, Qt::DirectConnection);
    io_thread_->start();
    db_thread_->start();
    ctp_thread_->start();
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

    dbservice_->init();
}

void ServiceMgr::dbThreadFinished()
{
    checkCurrentOn(DB);

    dbservice_->shutdown();
    dbservice_->moveToThread(ui_thread_);
}

void ServiceMgr::ctpThreadStarted()
{
    checkCurrentOn(CTP);

    ctpCmdMgr_->init();
    ctpMgr_->init();
    dataPump_->init();
}

void ServiceMgr::ctpThreadFinished()
{
    checkCurrentOn(CTP);

    dataPump_->shutdown();
    dataPump_->moveToThread(ui_thread_);

    ctpMgr_->shutdown();
    ctpMgr_->moveToThread(ui_thread_);

    ctpCmdMgr_->shutdown();
    ctpCmdMgr_->moveToThread(ui_thread_);
}

void ServiceMgr::shutdown()
{
    if (shutdown_ == true) {
        qFatal("shutdown_ == true");
        return;
    }

    ctp_thread_->quit();
    ctp_thread_->wait();
    delete ctp_thread_;
    ctp_thread_ = nullptr;

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

    delete dbservice_;
    dbservice_ = nullptr;

    delete dataPump_;
    dataPump_ = nullptr;

    delete ctpMgr_;
    ctpMgr_ = nullptr;

    delete ctpCmdMgr_;
    ctpCmdMgr_ = nullptr;

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

CtpCmdMgr* ServiceMgr::ctpCmdMgr()
{
    check();

    return this->ctpCmdMgr_;
}

CtpMgr* ServiceMgr::ctpMgr()
{
    check();

    return this->ctpMgr_;
}

DataPump* ServiceMgr::dataPump()
{
    check();

    return this->dataPump_;
}

DbService* ServiceMgr::dbService()
{
    check();

    return this->dbservice_;
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
    if (p == ServiceMgr::CTP) {
        return this->ctp_thread_;
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

    if (p == ServiceMgr::CTP && cur == ctp_thread_) {
        return true;
    }

    if (p == ServiceMgr::EXTERNAL) {
        if (cur != ui_thread_ && cur != db_thread_ && cur != io_thread_ && cur != ctp_thread_) {
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
