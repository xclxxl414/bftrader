#include "gatewaymgr.h"
#include "mdsm.h"
#include "profile.h"
#include "servicemgr.h"
#include "tdsm.h"
#include <windows.h>

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // cmdRunnder
    cmdRunnerTimer_ = new QTimer;
    cmdRunnerTimer_->setInterval(100);
    QObject::connect(cmdRunnerTimer_, &QTimer::timeout, this, &GatewayMgr::onRunCmdInterval);
    cmdRunnerTimer_->start();

    // qRegisterMetaType
    qRegisterMetaType<BfAccountData>("BfAccountData");
    qRegisterMetaType<BfPositionData>("BfPositionData");
    qRegisterMetaType<BfOrderData>("BfOrderData");
    qRegisterMetaType<BfTradeData>("BfTradeData");
    qRegisterMetaType<BfSendOrderReq>("BfSendOrderReq");
    qRegisterMetaType<BfCancelOrderReq>("BfCancelOrderReq");
    qRegisterMetaType<BfConnectReq>("BfConnectReq");

    // gotContracts
    QObject::connect(this, &GatewayMgr::gotContracts, this, &GatewayMgr::onGotContracts);
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    cmdRunnerTimer_->stop();
    delete cmdRunnerTimer_;
    cmdRunnerTimer_ = nullptr;

    freeContracts();
    freeRingBuffer();
}

void GatewayMgr::showVersion()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    BfInfo(QString("mdapi version: ") + MdSm::version());
    BfInfo(QString("tdapi version: ") + TdSm::version());
}

void GatewayMgr::onMdSmStateChanged(int state)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (!mdsm_) {
        BfDebug("mdsm freed,ingore onMdSmStateChanged:%d", state);
        return;
    }

    if (state == MDSM_CONNECTED) {
        mdsm_logined_ = false;
        if (autoLoginMd_) {
            mdsm_->login(1000);
        } else {
            mdsm_->stop();
        }
    }
    if (state == MDSM_DISCONNECTED) {
        mdsm_logined_ = false;
        if (!autoLoginMd_) {
            mdsm_->stop();
        } else {
            BfInfo("waiting for mdapi auto-reconnect......");
        }
    }
    if (state == MDSM_LOGINED) {
        mdsm_logined_ = true;
        tryStartSubscrible();
    }
    if (state == MDSM_LOGINFAIL) {
        if (autoLoginMd_) {
            BfInfo("mdsm login fail,try again 1 minute later");
            mdsm_->login(60 * 1000);
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

void GatewayMgr::onTdSmStateChanged(int state)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (!tdsm_) {
        BfDebug("tdsm freed,ingore onTdSmStateChanged:%d", state);
        return;
    }

    if (state == TDSM_CONNECTED) {
        tdsm_logined_ = false;
        if (autoLoginTd_) {
            tdsm_->login(1000);
        } else {
            tdsm_->stop();
        }
    }
    if (state == TDSM_DISCONNECTED) {
        resetCmds();
        tdsm_logined_ = false;
        if (!autoLoginTd_) {
            tdsm_->stop();
        } else {
            BfInfo("waiting for tdapi auto-reconnect......");
        }
    }
    if (state == TDSM_LOGINED) {
        tdsm_logined_ = true;
        tryStartSubscrible();
    }
    if (state == TDSM_LOGINFAIL) {
        if (autoLoginTd_) {
            BfInfo("tdsm login fail,try again 1 minute later");
            tdsm_->login(60 * 1000);
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

void GatewayMgr::start(QString password)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // check
    if (mdsm_ != nullptr || tdsm_ != nullptr) {
        BfDebug("mdsm_!= nullptr || tdsm_ != nullptr");
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

bool GatewayMgr::initMdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    mdsm_ = new MdSm;
    bool res = mdsm_->init(profile()->get("userId").toString(), password_,
        profile()->get("brokerId").toString(), profile()->get("frontMd").toString(), Profile::flowPathMd());
    if (!res) {
        delete mdsm_;
        mdsm_ = nullptr;
        BfInfo("invalid parameter,check please");
        return false;
    }
    return true;
}

void GatewayMgr::startMdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // go...
    QObject::connect(mdsm_, &MdSm::statusChanged, this, &GatewayMgr::onMdSmStateChanged);

    autoLoginMd_ = true;
    mdsm_->start();
}

bool GatewayMgr::initTdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    tdsm_ = new TdSm;
    bool res = tdsm_->init(profile()->get("userId").toString(), password_,
        profile()->get("brokerId").toString(), profile()->get("frontTd").toString(),
        Profile::flowPathTd(), profile()->get("idPrefixList").toString());
    if (!res) {
        delete tdsm_;
        tdsm_ = nullptr;
        BfInfo("invalid parameter,check please");
        return false;
    }
    return true;
}

void GatewayMgr::startTdSm()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // go...
    QObject::connect(tdsm_, &TdSm::statusChanged, this, &GatewayMgr::onTdSmStateChanged);

    autoLoginTd_ = true;
    tdsm_->start();
}

void GatewayMgr::tryStartSubscrible()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (mdsm_logined_ && tdsm_logined_) {
        emit tradeWillBegin();
        //函数开始执行时候才resetData
        tdsm_->queryInstrument(1000);
    }
    if (tdsm_ == nullptr) {
        if (!initTdSm()) {
            qFatal("initTdSm == false");
        }
        startTdSm();
    }
}

void GatewayMgr::stop()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // check
    if (mdsm_ == nullptr && tdsm_ == nullptr) {
        BfDebug("mdsm_ == nullptr && tdsm_ == nullptr");
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
            tdsm_->logout(0);
        } else {
            tdsm_->stop();
        }
    }
}

void GatewayMgr::onGotContracts(QStringList ids, QStringList idsAll)
{
    BfDebug(__FUNCTION__);

    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // 保存ids，便于枚举=
    ids_ = ids;
    ids_.sort();
    ids_all_ = idsAll;
    ids_all_.sort();

    // mdapi开始订阅=
    mdsm_->subscrible(ids, 0);

    // tdapi开始确认账单=
    tdsm_->reqSettlementInfoConfirm(0);
}

bool GatewayMgr::running()
{
    if (tdsm_ || mdsm_) {
        return true;
    }
    return false;
}

void* GatewayMgr::getLatestTick(QString id)
{
    auto rb = getRingBuffer(id);
    return rb->get(rb->head());
}

void* GatewayMgr::getPreLatestTick(QString id)
{
    auto rb = getRingBuffer(id);
    int preIndex = rb->head() - 1;
    if (preIndex < 0) {
        preIndex += rb->count();
    }
    return rb->get(preIndex);
}

void GatewayMgr::resetData()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    ids_.clear();
    ids_all_.clear();
    tdsm_->resetData(); //contract
    mdsm_->resetData(); //ringbuffer
}

QString GatewayMgr::genOrderId()
{
    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::genOrderId,please login first");
        return "888.888.888";
    }

    return tdsm_->genBfOrderId();
}

Logger* GatewayMgr::logger()
{
    return g_sm->logger();
}

Profile* GatewayMgr::profile()
{
    return g_sm->profile();
}

void GatewayMgr::queryAccount()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::queryAccount,please login first");
        return;
    }
    tdsm_->queryAccount(0);
}

void GatewayMgr::queryPosition()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::queryPosition,please login first");
        return;
    }
    tdsm_->queryPosition(0);
}

QStringList GatewayMgr::getIds()
{
    return ids_;
}

QStringList GatewayMgr::getIdsAll()
{
    return ids_all_;
}

void GatewayMgr::freeContracts()
{
    auto contract_list = contracts_.values();
    for (int i = 0; i < contract_list.length(); i++) {
        auto contract = contract_list.at(i);
        delete contract;
    }
    contracts_.clear();
}

void* GatewayMgr::getContract(QString id)
{
    auto contract = contracts_.value(id);
    if (contract == nullptr) {
        qFatal("contract == nullptr");
    }

    return contract;
}

void GatewayMgr::insertContract(QString id, void* contract)
{
    auto oldVal = contracts_.value(id);
    if (oldVal != nullptr) {
        qFatal("oldVal != nullptr");
    }
    contracts_[id] = contract;
}

RingBuffer* GatewayMgr::getRingBuffer(QString id)
{
    RingBuffer* rb = rbs_.value(id);
    if (rb == nullptr) {
        qFatal("rb == nullptr");
    }

    return rb;
}

void GatewayMgr::initRingBuffer(int itemLen, QStringList ids)
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

void GatewayMgr::freeRingBuffer()
{
    auto rb_list = rbs_.values();
    for (int i = 0; i < rb_list.length(); i++) {
        RingBuffer* rb = rb_list.at(i);
        rb->free();
        delete rb;
    }
    rbs_.clear();
}

void GatewayMgr::onRunCmdInterval()
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
    if (cmd->fn(++reqId_) == -3) {
        cmd->expires = curTick + 1000;
        BfError("sendcmd toofast,reqId=%d", reqId_);
        emit gotGatewayError(-3, "sendmsg too fast", QString().sprintf("reqId=%d", reqId_));
        return;
    }

    // 消费掉这个cmd=
    cmds_.dequeue();
    delete cmd;
}

void GatewayMgr::runCmd(CtpCmd* cmd)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (cmd->delayTick == 0) {
        int result = cmd->fn(++reqId_);
        if (result == -3) {
            BfError("sendcmd toofast,reqId=%d", reqId_);
            emit gotGatewayError(-3, "sendmsg too fast", QString().sprintf("reqId=%d", reqId_));
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
void GatewayMgr::resetCmds()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (cmds_.length()) {
        BfDebug(__FUNCTION__);

        for (auto cmd : cmds_) {
            delete cmd;
        }
        cmds_.clear();
    }
}

void GatewayMgr::sendOrder(const BfSendOrderReq& req)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::sendOrder,please login first");
        return;
    }

    tdsm_->sendOrder(0, req);
}

void GatewayMgr::sendOrderWithId(QString bfOrderId, const BfSendOrderReq& req)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::sendOrder,please login first");
        return;
    }

    tdsm_->sendOrder(0, bfOrderId, req);
}

void GatewayMgr::cancelOrder(const BfCancelOrderReq& req)
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::cancelOrder,please login first");
        return;
    }

    tdsm_->cancelOrder(0, req);
}

void GatewayMgr::queryOrders()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (tdsm_ == nullptr) {
        BfInfo("GatewayMgr::queryOrders,please login first");
        return;
    }
    tdsm_->queryOrders(0);
}
