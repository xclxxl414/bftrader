#include "ctpmgr.h"
#include "logger.h"
#include "mdsm.h"
#include "profile.h"
#include "servicemgr.h"
#include "tdsm.h"
#include <windows.h>

CtpMgr::CtpMgr(QObject* parent)
    : QObject(parent)
{
}

void CtpMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    cmdRunnerTimer_ = new QTimer;
    cmdRunnerTimer_->setInterval(100);
    QObject::connect(cmdRunnerTimer_, &QTimer::timeout, this, &CtpMgr::onRunCmdInterval);
    cmdRunnerTimer_->start();
}

void CtpMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    cmdRunnerTimer_->stop();
    delete cmdRunnerTimer_;
    cmdRunnerTimer_ = nullptr;

    freeContracts();
    freeRingBuffer();
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
        resetCmds();
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
        resetCmds();
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
    QObject::connect(mdsm_, &MdSm::statusChanged, this, &CtpMgr::onMdSmStateChanged);
    QObject::connect(mdsm_, &MdSm::gotTick, this, &CtpMgr::gotTick);

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
    QObject::connect(tdsm_, &TdSm::statusChanged, this, &CtpMgr::onTdSmStateChanged);
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
        emit this->tradeWillBegin();
        tdsm_->resetData();
        mdsm_->resetData();
        tdsm_->queryInstrument(1000, "");
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

void* CtpMgr::getLatestTick(QString id)
{
    auto rb = getRingBuffer(id);
    return rb->get(rb->head());
}

void* CtpMgr::getPreLatestTick(QString id)
{
    auto rb = getRingBuffer(id);
    int preIndex = rb->head() - 1;
    if (preIndex < 0) {
        preIndex += rb->count();
    }
    return rb->get(preIndex);
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
        logger()->info("CtpMgr::queryAccount,please login first");
        return;
    }
    tdsm_->queryAccount(0, "");
}

void CtpMgr::freeContracts()
{
    auto contract_list = contracts_.values();
    for (int i = 0; i < contract_list.length(); i++) {
        auto contract = contract_list.at(i);
        delete contract;
    }
    contracts_.clear();
}

void* CtpMgr::getContract(QString id)
{
    auto contract = contracts_.value(id);
    if (contract == nullptr) {
        qFatal("contract == nullptr");
    }

    return contract;
}

void CtpMgr::insertContract(QString id, void* contract)
{
    auto oldVal = contracts_.value(id);
    if (oldVal != nullptr) {
        qFatal("oldVal != nullptr");
    }
    contracts_[id] = contract;
}

RingBuffer* CtpMgr::getRingBuffer(QString id)
{
    RingBuffer* rb = rbs_.value(id);
    if (rb == nullptr) {
        qFatal("rb == nullptr");
    }

    return rb;
}

void CtpMgr::initRingBuffer(int itemLen, QStringList ids)
{
    if (rbs_.count() != 0) {
        qFatal("rbs_.count() != 0");
    }

    for (auto id : ids) {
        RingBuffer* rb = new RingBuffer;
        rb->init(itemLen, ringBufferLen_);
        rbs_.insert(id, rb);
    }
}

void CtpMgr::freeRingBuffer()
{
    auto rb_list = rbs_.values();
    for (int i = 0; i < rb_list.length(); i++) {
        RingBuffer* rb = rb_list.at(i);
        rb->free();
        delete rb;
    }
    rbs_.clear();
}

void CtpMgr::onRunCmdInterval()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (cmds_.length() == 0) {
        return;
    }

    CtpCmd* cmd = cmds_.head();

    // 检查时间是否到了=
    quint32 curTick = ::GetTickCount();
    if (curTick < cmd->expires) {
        return;
    }

    // 流控了就一秒后重试=
    if (cmd->fn(++reqId_, cmd->robotId) == -3) {
        cmd->expires = curTick + 1000;
        g_sm->logger()->info(QString().sprintf("发包太快，reqId=%d", reqId_));
        return;
    }

    // 消费掉这个cmd=
    cmds_.dequeue();
    delete cmd;
}

void CtpMgr::runCmd(CtpCmd* cmd)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (cmd->delayTick == 0) {
        int result = cmd->fn(++reqId_, cmd->robotId);
        if (result == -3) {
            g_sm->logger()->info(QString().sprintf("发包太快，reqId=%d", reqId_));
            cmd->expires = ::GetTickCount() + 1000;
            cmds_.append(cmd);
        } else {
            delete cmd;
        }
    } else {
        cmd->expires = ::GetTickCount() + cmd->delayTick;
        cmds_.append(cmd);
    }
}
void CtpMgr::resetCmds()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    logger()->info(__FUNCTION__);

    for (auto cmd : cmds_) {
        delete cmd;
    }
    cmds_.clear();
}
