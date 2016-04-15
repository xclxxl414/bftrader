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
    if (state == MDSM_CONNECTED) {
        if (autoLoginMd_) {
            mdsm_->login(1000, "");
        }
    }
    if (state == MDSM_DISCONNECTED) {
        mdsm_logined_ = false;
    }
    if (state == MDSM_LOGINED) {
        mdsm_logined_ = true;
        tryStartSubscrible();
    }
    if (state == MDSM_LOGINFAIL) {
        logger()->info("mdsm login fail,try again 1 minute later");
        mdsm_->login(60 * 1000, "");
    }
    if (state == MDSM_STOPPED) {
        delete mdsm_;
        mdsm_ = nullptr;
        mdsm_logined_ = false;
    }
}

void CtpMgr::onTdSmStateChanged(int state)
{
    if (state == TDSM_CONNECTED) {
        if (autoLoginTd_) {
            tdsm_->login(1000, "");
        }
    }
    if (state == TDSM_DISCONNECTED) {
        tdsm_logined_ = false;
    }
    if (state == TDSM_LOGINED) {
        tdsm_logined_ = true;
        tryStartSubscrible();
    }
    if (state == TDSM_LOGINFAIL) {
        logger()->info("tdsm login fail,try again 1 minute later");
        tdsm_->login(60 * 1000, "");
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
        delete tdsm_;
        tdsm_ = nullptr;
        tdsm_logined_ = false;
    }
}

void CtpMgr::start(QString password)
{
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
    mdsm_ = new MdSm;
    bool res = mdsm_->init(profile()->get("userId").toString(), password_,
        profile()->get("brokerId").toString(), profile()->get("frontMd").toString(), Profile::flowPathMd());
    if (!res) {
        delete mdsm_;
        mdsm_ = nullptr;
        logger()->info("参数无效，请核对参数=");
        return false;
    }
    return true;
}

void CtpMgr::startMdSm()
{
    // go...
    QObject::connect(mdsm_, &MdSm::statusChanged, this, &CtpMgr::onMdSmStateChanged);

    autoLoginMd_ = true;
    mdsm_->start();
}

bool CtpMgr::initTdSm()
{
    tdsm_ = new TdSm;
    bool res = tdsm_->init(profile()->get("userId").toString(), password_,
        profile()->get("brokerId").toString(), profile()->get("frontTd").toString(),
        Profile::flowPathTd(), profile()->get("idPrefixList").toString());
    if (!res) {
        delete tdsm_;
        tdsm_ = nullptr;
        logger()->info("参数无效，请核对参数=");
        return false;
    }
    return true;
}

void CtpMgr::startTdSm()
{
    // go...
    QObject::connect(tdsm_, &TdSm::statusChanged, this, &CtpMgr::onTdSmStateChanged);
    QObject::connect(tdsm_, &TdSm::gotInstruments, this, &CtpMgr::onGotInstruments);

    autoLoginTd_ = true;
    tdsm_->start();
}

void CtpMgr::tryStartSubscrible()
{
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
    // check
    if (mdsm_ == nullptr) {
        logger()->info("mdsm_ == nullptr");
        return;
    }
    mdsm_->stop();

    if (tdsm_) {
        tdsm_->stop();
    }
}

void CtpMgr::onGotInstruments(QStringList ids)
{
    //退出td
    autoLoginTd_ = false;
    tdsm_->logout(0, "");

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

Logger* CtpMgr::logger()
{
    return g_sm->logger();
}

Profile* CtpMgr::profile()
{
    return g_sm->profile();
}
