#include "ctpmgr.h"
#include "logger.h"
#include "mdsm.h"
#include "profile.h"
#include "servicemgr.h"
#include "tdsm.h"

CtpMgr::CtpMgr(QObject* parent)
    : QObject(parent)
{
}

void CtpMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

void CtpMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
}

void CtpMgr::showVersion()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    g_sm->logger()->info(QString("mdapi version: ") + MdSm::version());
    g_sm->logger()->info(QString("tdapi version: ") + TdSm::version());
}

void CtpMgr::onMdSmStateChanged(int state)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (!mdsm_) {
        logger()->info(QString().sprintf("mdsm freed,ingore onMdSmStateChanged:%d", state));
        return;
    }

    if (state == MDSM_CONNECTED) {
        mdsm_logined_ = false;
        if (autoLoginMd_) {
            mdsm_->login(1000, "");
        } else {
            mdsm_->stop();
        }
    }
    if (state == MDSM_DISCONNECTED) {
        mdsm_logined_ = false;
        if (!autoLoginMd_) {
            mdsm_->stop();
        }
    }
    if (state == MDSM_LOGINED) {
        mdsm_logined_ = true;
        tryStartSubscrible();
    }
    // todo(hege):1分钟内如果stop了，就这里的定时器有问题，还是需要做一个队列=
    if (state == MDSM_LOGINFAIL) {
        if (autoLoginMd_) {
            logger()->info("mdsm login fail,try again 1 minute later");
            mdsm_->login(60 * 1000, "");
        } else {
            mdsm_->stop();
        }
    }
    if (state == MDSM_STOPPED) {
        mdsm_logined_ = false;
        delete mdsm_;
        mdsm_ = nullptr;
        mdsm_logined_ = false;
    }
}

void CtpMgr::onTdSmStateChanged(int state)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (!tdsm_) {
        logger()->info(QString().sprintf("tdsm freed,ingore onTdSmStateChanged:%d", state));
        return;
    }

    if (state == TDSM_CONNECTED) {
        tdsm_logined_ = false;
        if (autoLoginTd_) {
            tdsm_->login(1000, "");
        } else {
            tdsm_->stop();
        }
    }
    if (state == TDSM_DISCONNECTED) {
        tdsm_logined_ = false;
        if (!autoLoginTd_) {
            tdsm_->stop();
        }
    }
    if (state == TDSM_LOGINED) {
        tdsm_logined_ = true;
        tryStartSubscrible();
    }
    if (state == TDSM_LOGINFAIL) {
        if (autoLoginTd_) {
            logger()->info("tdsm login fail,try again 1 minute later");
            tdsm_->login(60 * 1000, "");
        } else {
            tdsm_->stop();
        }
    }
    if (state == TDSM_LOGOUTED) {
        tdsm_logined_ = false;
        tdsm_->stop();
    }
    if (state == TDSM_LOGOUTFAIL) {
        tdsm_logined_ = false;
        tdsm_->stop();
    }
    if (state == TDSM_STOPPED) {
        tdsm_logined_ = false;
        delete tdsm_;
        tdsm_ = nullptr;
        tdsm_logined_ = false;
    }
}

void CtpMgr::start(QString password)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // check
    if (mdsm_ != nullptr || tdsm_ != nullptr) {
        logger()->info("mdsm_!= nullptr || tdsm_ != nullptr");
        return;
    }

    // init mdsm
    password_ = password;

    if (!initMdSm()) {
        return;
    }

    if (!initTdSm()) {
        delete mdsm_;
        mdsm_ = nullptr;
        return;
    }

    startMdSm();
    startTdSm();
    return;
}

bool CtpMgr::initMdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    mdsm_ = new MdSm;
    bool res = mdsm_->init(profile()->get("userId").toString(), password_,
        profile()->get("brokerId").toString(), profile()->get("frontMd").toString(), Profile::flowPathMd());
    if (!res) {
        delete mdsm_;
        mdsm_ = nullptr;
        logger()->info("invalid parameter,check please");
        return false;
    }
    return true;
}

void CtpMgr::startMdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // go...
    QObject::connect(mdsm_, &MdSm::statusChanged, this, &CtpMgr::onMdSmStateChanged, Qt::QueuedConnection);
    QObject::connect(mdsm_, &MdSm::gotTick, this, &CtpMgr::gotTick);
    QObject::connect(mdsm_, &MdSm::tradeClosed, this, &CtpMgr::tradeClosed);

    autoLoginMd_ = true;
    mdsm_->start();
}

bool CtpMgr::initTdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    tdsm_ = new TdSm;
    bool res = tdsm_->init(profile()->get("userId").toString(), password_,
        profile()->get("brokerId").toString(), profile()->get("frontTd").toString(),
        Profile::flowPathTd(), profile()->get("idPrefixList").toString());
    if (!res) {
        delete tdsm_;
        tdsm_ = nullptr;
        logger()->info("invalid parameter,check please");
        return false;
    }
    return true;
}

void CtpMgr::startTdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // go...
    QObject::connect(tdsm_, &TdSm::statusChanged, this, &CtpMgr::onTdSmStateChanged, Qt::QueuedConnection);
    QObject::connect(tdsm_, &TdSm::gotInstruments, this, &CtpMgr::onGotInstruments);
    QObject::connect(tdsm_, &TdSm::gotInstruments, this, &CtpMgr::gotInstruments);
    QObject::connect(tdsm_, &TdSm::gotAccount, this, &CtpMgr::gotAccount);

    autoLoginTd_ = true;
    tdsm_->start();
}

void CtpMgr::tryStartSubscrible()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (mdsm_logined_ && tdsm_logined_) {
        tdsm_->queryInstrument(0, "");
    }
    if (tdsm_ == nullptr) {
        if (!initTdSm()) {
            qFatal("initTdSm == false");
        }
        startTdSm();
    }
}

void CtpMgr::stop()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // check
    if (mdsm_ == nullptr && tdsm_ == nullptr) {
        logger()->info("mdsm_ == nullptr && tdsm_ == nullptr");
        return;
    }
    if (mdsm_) {
        autoLoginMd_ = false;
        mdsm_->stop();
    }

    if (tdsm_) {
        autoLoginTd_ = false;
        if (tdsm_logined_) {
            //先logout，然后自动退出=
            tdsm_->logout(0, "");
        } else {
            tdsm_->stop();
        }
    }
}

void CtpMgr::onGotInstruments(QStringList ids)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // 开始订阅=
    mdsm_->subscrible(ids, 0, "");
}

bool CtpMgr::running()
{
    if (tdsm_ || mdsm_) {
        return true;
    }
    return false;
}

void* CtpMgr::getContract(QString id)
{
    if (tdsm_) {
        return tdsm_->getContract(id);
    }

    logger()->info("tdsm_ == nullptr,please login first");
    return nullptr;
}

void* CtpMgr::getLatestTick(QString id)
{
    if (mdsm_) {
        return mdsm_->getLatestTick(id);
    }

    logger()->info("mdsm_ == nullptr,please login first");
    return nullptr;
}

void* CtpMgr::getPreLatestTick(QString id)
{
    if (mdsm_) {
        return mdsm_->getPreLatestTick(id);
    }

    logger()->info("mdsm_ == nullptr,please login first");
    return nullptr;
}

Logger* CtpMgr::logger()
{
    return g_sm->logger();
}

Profile* CtpMgr::profile()
{
    return g_sm->profile();
}

void CtpMgr::queryAccount()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        logger()->info("please login first");
        return;
    }
    tdsm_->queryAccount(0, "");
}
