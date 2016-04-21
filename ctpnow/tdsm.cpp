#include "tdsm.h"
#include "ThostFtdcTraderApi.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "logger.h"
#include "servicemgr.h"
#include <QDir>

///////////
class TdSmSpi : public CThostFtdcTraderSpi {
public:
    explicit TdSmSpi(TdSm* sm)
        : sm_(sm)
    {
    }

private:
    void OnFrontConnected() override
    {
        info(__FUNCTION__);
        emit sm()->statusChanged(TDSM_CONNECTED);
    }

    // 客户端与服务端连接断开后，交易接口会自动尝试重新连接，频率是每 5 秒一次。(CTPSDK)
    // 如果网络异常，会直接调用OnFrontDisconnected，需要重置状态数据=
    // 网络错误当再次恢复时候，会自动重连重新走OnFrontConnected
    // logout也会导致一次disconnected+connected，0x1001
    void OnFrontDisconnected(int nReason) override
    {
        info(QString().sprintf("TdSmSpi::OnFrontDisconnected,nReason=0x%x", nReason));

        emit sm()->statusChanged(TDSM_DISCONNECTED);
    }

    // errorId=3，msg=CTP:不合法的登陆=
    // errorId=7，msg=CTP:还没有初始化=
    // errorId=8,msg=CTP:前置不活跃=
    // 1. 并不是connected就可以登陆的=
    // 2. 如果connected后不能登录，对于7，会过一会来一个disconnected+connected，所以不用处理=
    // 3. 发现对于errorid=7，不一定会disconnect后再connected，需要自己去发包=
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                emit sm()->statusChanged(TDSM_LOGINFAIL);
            } else {
                orderRef_ = 0;
                frontId_ = pRspUserLogin->FrontID;
                sessionId_ = pRspUserLogin->SessionID;

                emit sm()->statusChanged(TDSM_LOGINED);
            }
        }
    }

    // logout不一定有logout的rep的，可能是直接disconnect+connect，所以在connect里面判断一下是否需要auologin，如果不是
    // 就是手动停止了，stop掉=
    void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                emit sm()->statusChanged(TDSM_LOGOUTFAIL);
            } else {
                emit sm()->statusChanged(TDSM_LOGOUTED);
            }
        }
    }

    //出现了一次queryinstruments错误，打印详细信息=
    void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        if (bIsLast) {
            info(__FUNCTION__);
            isErrorRsp(pRspInfo, nRequestID);
        }
    }

    // 可能有多次回调=
    void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        QString id;
        QString prefix;
        if (!idPrefixList_.length()) {
            QString prefixlist = sm()->idPrefixList_;
            idPrefixList_ = prefixlist.split(";");
        }
        if (!isErrorRsp(pRspInfo, nRequestID) && pInstrument) {
            id = pInstrument->InstrumentID;
            if (id.length() <= 6) {
                QString low_id = id.toLower();
                for (int i = 0; i < idPrefixList_.length(); i++) {
                    prefix = idPrefixList_.at(i);
                    if (low_id.startsWith(prefix)) {
                        ids_ << id;

                        // 保存一份供查询=
                        CThostFtdcInstrumentField* contract = new CThostFtdcInstrumentField();
                        memcpy(contract, pInstrument, sizeof(CThostFtdcInstrumentField));
                        g_sm->ctpMgr()->insertContract(id, contract);
                        break;
                    }
                }
            }
        }

        if (bIsLast && ids_.length()) {
            QString ids;
            for (auto id : ids_) {
                ids = ids + id + ";";
            }
            info(QString().sprintf("total got ids:%d,%s", ids_.length(), ids.toUtf8().constData()));
            emit g_sm->ctpMgr()->gotInstruments(ids_);
        }
    }

    // 请求查询资金账户响应=
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                return;
            } else {
                BfAccountData account;
                //账户代码=
                account.set_accountid(pTradingAccount->AccountID);
                //数值相关=
                account.set_prebalance(pTradingAccount->PreBalance);
                double balance = pTradingAccount->PreBalance - pTradingAccount->PreCredit - pTradingAccount->PreMortgage + pTradingAccount->Mortgage - pTradingAccount->Withdraw + pTradingAccount->Deposit + pTradingAccount->CloseProfit + pTradingAccount->PositionProfit + pTradingAccount->CashIn - pTradingAccount->Commission;
                account.set_balance(balance);
                account.set_balance(pTradingAccount->Balance);
                account.set_available(pTradingAccount->Available);
                account.set_commission(pTradingAccount->Commission);
                double frozenMargin = pTradingAccount->FrozenMargin;
                if ((frozenMargin > 0) && (balance > 0)) {
                    frozenMargin = (frozenMargin * 100.0) / balance;
                }
                account.set_frozenmargin(frozenMargin);
                account.set_closeprofit(pTradingAccount->CloseProfit);
                account.set_positionprofit(pTradingAccount->PositionProfit);

                emit g_sm->ctpMgr()->gotAccount(account);
            }
        }
    }

    // 请求查询投资者持仓响应，可能有多个回调=
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        if (!isErrorRsp(pRspInfo, nRequestID) && pInvestorPosition) {
            BfPositionData pos;

            // 保存代码=
            pos.set_symbol(pInvestorPosition->InstrumentID);
            pos.set_direction(CtpUtils::translatePosiDirection(pInvestorPosition->PosiDirection));

            // 冻结=
            if (pos.direction() == DIRECTION_LONG || pos.direction() == DIRECTION_NET) {
                pos.set_frozen(pInvestorPosition->LongFrozen);
            } else if (pos.direction() == DIRECTION_SHORT) {
                pos.set_frozen(pInvestorPosition->ShortFrozen);
            }

            // 持仓量=
            pos.set_position(pInvestorPosition->Position);
            pos.set_ydposition(pInvestorPosition->YdPosition);

            // 均价=
            if (pos.position() > 0) {
                pos.set_price(pInvestorPosition->PositionCost / pos.position());
            }

            emit g_sm->ctpMgr()->gotPosition(pos);
        }

        if (bIsLast) {
            info(__FUNCTION__);
        }
    }

    // 投资者结算结果确认响应=
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                return;
            } else {
            }
        }
    }

    // 请求查询报单响应，可能有多次回调=
    virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        if (!isErrorRsp(pRspInfo, nRequestID) && pOrder) {
            int orderRef = QString(pOrder->OrderRef).toInt();
            QString bfOrderId = CtpUtils::formatBfOrderId(pOrder->FrontID, pOrder->SessionID, orderRef);

            BfOrderData order;
            // 保存代码和报单号=
            order.set_exchange(pOrder->ExchangeID);
            order.set_symbol(pOrder->InstrumentID);
            order.set_bforderid(bfOrderId.toStdString());
            order.set_sysorderid(pOrder->OrderSysID);
            order.set_direction(CtpUtils::translateDirection(pOrder->Direction)); //方向=
            order.set_offset(CtpUtils::translateOffset(pOrder->CombOffsetFlag[0])); //开平=
            order.set_status(CtpUtils::translateStatus(pOrder->OrderStatus)); //状态=

            //价格、报单量等数值=
            order.set_price(pOrder->LimitPrice);
            order.set_totalvolume(pOrder->VolumeTotalOriginal);
            order.set_tradedvolume(pOrder->VolumeTraded);
            order.set_insertdate(pOrder->InsertDate);
            order.set_inserttime(pOrder->InsertTime);
            order.set_canceltime(pOrder->CancelTime);

            emit g_sm->ctpMgr()->gotOrder(order);
        }

        if (bIsLast) {
            info(__FUNCTION__);
        }
    }

    // 报单录入请求响应=
    // 发单错误（柜台）=
    void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID) && pInputOrder) {
                int orderRef = QString(pInputOrder->OrderRef).toInt();
                QString bfOrderId = CtpUtils::formatBfOrderId(frontId_, sessionId_, orderRef);
                info(QString().sprintf("OnRspOrderInsert: bfOrderId = %s", bfOrderId.toStdString().c_str()));
                return;
            } else {
            }
        }
    }

    // 报单录入错误回报=
    // 发单错误回报（交易所）=
    void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo) override
    {
        info(__FUNCTION__);
        if (true) {
            if (isErrorRsp(pRspInfo, 0) && pInputOrder) {
                int orderRef = QString(pInputOrder->OrderRef).toInt();
                QString bfOrderId = CtpUtils::formatBfOrderId(frontId_, sessionId_, orderRef);
                info(QString().sprintf("OnErrRtnOrderInsert: bfOrderId = %s", bfOrderId.toStdString().c_str()));
                return;
            } else {
            }
        }
    }

    // 报单操作请求响应=
    // 撤单错误（柜台）=
    void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID) && pInputOrderAction) {
                int orderRef = QString(pInputOrderAction->OrderRef).toInt();
                QString bfOrderId = CtpUtils::formatBfOrderId(pInputOrderAction->FrontID, pInputOrderAction->SessionID, orderRef);
                info(QString().sprintf("OnRspOrderAction: bfOrderId = %s", bfOrderId.toStdString().c_str()));
                return;
            } else {
            }
        }
    }

    // 报单操作错误回报=
    // 撤单错误回报（交易所）=
    void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo) override
    {
        info(__FUNCTION__);
        if (true) {
            if (isErrorRsp(pRspInfo, 0) && pOrderAction) {
                int orderRef = QString(pOrderAction->OrderRef).toInt();
                QString bfOrderId = CtpUtils::formatBfOrderId(pOrderAction->FrontID, pOrderAction->SessionID, orderRef);
                info(QString().sprintf("OnErrRtnOrderAction: bfOrderId = %s", bfOrderId.toStdString().c_str()));
                return;
            } else {
            }
        }
    }

    // 报单通知=
    virtual void OnRtnOrder(CThostFtdcOrderField* pOrder)
    {
        info(__FUNCTION__);

        int orderRef = QString(pOrder->OrderRef).toInt();
        orderRef_ = qMax(orderRef, orderRef_);

        QString bfOrderId = CtpUtils::formatBfOrderId(pOrder->FrontID, pOrder->SessionID, orderRef);
        info(QString().sprintf("OnRtnOrder: bfOrderId = %s,sysOrderId=%s", bfOrderId.toStdString().c_str(), pOrder->OrderSysID));

        BfOrderData order;
        // 保存代码和报单号=
        order.set_exchange(pOrder->ExchangeID);
        order.set_symbol(pOrder->InstrumentID);
        order.set_bforderid(bfOrderId.toStdString());
        order.set_sysorderid(pOrder->OrderSysID);
        order.set_direction(CtpUtils::translateDirection(pOrder->Direction)); //方向=
        order.set_offset(CtpUtils::translateOffset(pOrder->CombOffsetFlag[0])); //开平=
        order.set_status(CtpUtils::translateStatus(pOrder->OrderStatus)); //状态=

        //价格、报单量等数值=
        order.set_price(pOrder->LimitPrice);
        order.set_totalvolume(pOrder->VolumeTotalOriginal);
        order.set_tradedvolume(pOrder->VolumeTraded);
        order.set_insertdate(pOrder->InsertDate);
        order.set_inserttime(pOrder->InsertTime);
        order.set_canceltime(pOrder->CancelTime);

        emit g_sm->ctpMgr()->gotOrder(order);
    }

    // 成交通知=
    virtual void OnRtnTrade(CThostFtdcTradeField* pTrade)
    {
        info(__FUNCTION__);

        BfTradeData trade;

        // 保存代码和报单号=
        trade.set_exchange(pTrade->ExchangeID);
        trade.set_symbol(pTrade->InstrumentID);
        trade.set_sysorderid(pTrade->OrderSysID); //这里没有bfOrderId
        trade.set_tradeid(pTrade->TradeID);
        trade.set_direction(CtpUtils::translateDirection(pTrade->Direction)); //方向=
        trade.set_offset(CtpUtils::translateOffset(pTrade->OffsetFlag)); //开平=

        //价格、报单量等数值=
        trade.set_price(pTrade->Price);
        trade.set_volume(pTrade->Volume);
        trade.set_tradedate(pTrade->TradeDate);
        trade.set_tradetime(pTrade->TradeTime);

        emit g_sm->ctpMgr()->gotTrade(trade);
    }

private:
    bool isErrorRsp(CThostFtdcRspInfoField* pRspInfo, int reqId)
    {
        if (pRspInfo && pRspInfo->ErrorID != 0) {
            info(QString().sprintf("<==错误，reqid=%d,errorId=%d，msg=%s", reqId, pRspInfo->ErrorID, gbk2utf16(pRspInfo->ErrorMsg).toUtf8().constData()));
            return true;
        }
        return false;
    }

    TdSm* sm()
    {
        return sm_;
    }

    void resetData()
    {
        g_sm->checkCurrentOn(ServiceMgr::LOGIC);

        ids_.clear();
        idPrefixList_.clear();
        g_sm->ctpMgr()->freeContracts();
    }

    void info(QString msg)
    {
        g_sm->logger()->info(msg);
    }

    int getOrderRef()
    {
        orderRef_++;
        return orderRef_;
    }

    int getFrontId() { return frontId_; }
    int getSessionId() { return sessionId_; }

private:
    TdSm* sm_;
    QStringList ids_;
    QStringList idPrefixList_;
    int orderRef_ = 0;
    int frontId_ = 0;
    int sessionId_ = 0;

    friend TdSm;
};

///////////
TdSm::TdSm(QObject* parent)
    : QObject(parent)
{
}

TdSm::~TdSm()
{
}

bool TdSm::init(QString userId, QString password, QString brokerId, QString frontTd, QString flowPathTd, QString idPrefixList)
{
    userId_ = userId;
    password_ = password;
    brokerId_ = brokerId;
    frontTd_ = frontTd;
    flowPathTd_ = flowPathTd;
    idPrefixList_ = idPrefixList;

    // check
    if (userId_.length() == 0 || password_.length() == 0
        || brokerId_.length() == 0 || frontTd_.length() == 0 || flowPathTd_.length() == 0
        || idPrefixList_.length() == 0) {
        return false;
    }

    return true;
}

void TdSm::start()
{
    info(__FUNCTION__);

    if (tdapi_ != nullptr) {
        qFatal("tdapi_!=nullptr");
        return;
    }

    QDir dir;
    dir.mkpath(flowPathTd_);
    tdapi_ = CThostFtdcTraderApi::CreateFtdcTraderApi(flowPathTd_.toStdString().c_str());
    tdspi_ = new TdSmSpi(this);
    tdapi_->RegisterSpi(tdspi_);
    tdapi_->RegisterFront((char*)qPrintable(frontTd_));
    tdapi_->SubscribePublicTopic(THOST_TERT_QUICK);
    tdapi_->SubscribePrivateTopic(THOST_TERT_QUICK);
    tdapi_->Init();
}

void TdSm::stop()
{
    info(__FUNCTION__);

    if (tdapi_ == nullptr) {
        qFatal("tdapi_==nullptr");
        return;
    }

    this->tdapi_->RegisterSpi(nullptr);
    this->tdapi_->Release();

    this->tdapi_ = nullptr;
    delete this->tdspi_;
    this->tdspi_ = nullptr;
    emit this->statusChanged(TDSM_STOPPED);
}

void TdSm::info(QString msg)
{
    g_sm->logger()->info(msg);
}

QString TdSm::version()
{
    return CThostFtdcTraderApi::GetApiVersion();
}

void TdSm::resetData()
{
    this->tdspi_->resetData();
}

// 登录填userID，其他填investorid=
void TdSm::login(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcReqUserLoginField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.UserID, userId_.toStdString().c_str(), sizeof(req.UserID) - 1);
        strncpy(req.Password, password_.toStdString().c_str(), sizeof(req.Password) - 1);

        int result = tdapi_->ReqUserLogin(&req, reqId);
        info(QString().sprintf("CmdTdLogin,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

//目前，通过 ReqUserLogout 登出系统的话，会先将现有的连接断开，再重新建立一个新的连接(CTPSDK)
// logout之后会有一个disconnect/connect...先disableautologin
void TdSm::logout(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcUserLogoutField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.UserID, userId_.toStdString().c_str(), sizeof(req.UserID) - 1);

        int result = tdapi_->ReqUserLogout(&req, reqId);
        info(QString().sprintf("CmdTdLogout,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::queryInstrument(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        // 重置ctpmgr相关内存=
        g_sm->ctpMgr()->resetData();

        CThostFtdcQryInstrumentField req;
        memset(&req, 0, sizeof(req));

        int result = tdapi_->ReqQryInstrument(&req, reqId);
        info(QString().sprintf("CmdTdQueryInstrument,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::queryAccount(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcQryTradingAccountField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);

        int result = tdapi_->ReqQryTradingAccount(&req, reqId);
        info(QString().sprintf("CmdTdQueryInvestorPosition,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::reqSettlementInfoConfirm(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcSettlementInfoConfirmField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);

        int result = tdapi_->ReqSettlementInfoConfirm(&req, reqId);
        info(QString().sprintf("CmdTdReqSettlementInfoConfirm,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::queryPosition(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcQryInvestorPositionField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);

        int result = tdapi_->ReqQryInvestorPosition(&req, reqId);
        info(QString().sprintf("CmdTdReqQryInvestorPosition,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::sendOrder(unsigned int delayTick, QString robotId, const BfSendOrderReq& bfReq)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcInputOrderField req;
        memset(&req, 0, sizeof(req));

        int orderRef = tdspi_->getOrderRef();
        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);
        strncpy(req.OrderRef, QString::number(orderRef).toStdString().c_str(), sizeof(req.OrderRef) - 1);
        strncpy(req.InstrumentID, bfReq.symbol().c_str(), sizeof(req.InstrumentID) - 1);

        req.Direction = CtpUtils::translateDirection(bfReq.direction());
        req.CombOffsetFlag[0] = CtpUtils::translateOffset(bfReq.offset());
        req.OrderPriceType = CtpUtils::translatePriceType(bfReq.pricetype());
        req.LimitPrice = bfReq.price();
        req.VolumeTotalOriginal = bfReq.volume();

        req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation; // 投机单=
        req.ContingentCondition = THOST_FTDC_CC_Immediately; // 立即发单=
        req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose; // 非强平=
        req.IsAutoSuspend = 0; // 非自动挂起=
        req.TimeCondition = THOST_FTDC_TC_GFD; // 今日有效=
        req.VolumeCondition = THOST_FTDC_VC_AV; // 任意成交量=
        req.MinVolume = 1; // 最小成交量为1=

        int result = tdapi_->ReqOrderInsert(&req, reqId);
        QString bfOrderId = CtpUtils::formatBfOrderId(tdspi_->getFrontId(), tdspi_->getSessionId(), orderRef);
        info(QString().sprintf("CmdTdReqOrderInsert,bfOrderId=%s,reqId=%d,result=%d", bfOrderId.toStdString().c_str(), reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::cancelOrder(unsigned int delayTick, QString robotId, const BfCancelOrderReq& bfReq)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcInputOrderActionField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);

        strncpy(req.InstrumentID, bfReq.symbol().c_str(), sizeof(req.InstrumentID) - 1);
        strncpy(req.ExchangeID, bfReq.exchange().c_str(), sizeof(req.InstrumentID) - 1);

        // frontid+sessionid+orderref || exchangeid + ordersysid
        QString bfOrderId = bfReq.bforderid().c_str();
        int frontId, sessionId, orderRef;
        CtpUtils::translateBfOrderId(bfOrderId, frontId, sessionId, orderRef);

        strncpy(req.OrderRef, QString::number(orderRef).toStdString().c_str(), sizeof(req.OrderRef) - 1);
        req.FrontID = frontId;
        req.SessionID = sessionId;

        req.ActionFlag = THOST_FTDC_AF_Delete;

        int result = tdapi_->ReqOrderAction(&req, reqId);
        info(QString().sprintf("CmdTdReqOrderAction,bfOrderId=%s,reqId=%d,result=%d", bfOrderId.toStdString().c_str(), reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}

void TdSm::queryOrders(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcQryOrderField req;
        memset(&req, 0, sizeof(req));

        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);

        int result = tdapi_->ReqQryOrder(&req, reqId);
        info(QString().sprintf("CmdTdReqQryOrder,reqId=%d,result=%d", reqId, result));
        if (result == 0) {
            emit g_sm->ctpMgr()->requestSent(reqId, robotId);
        }
        return result;
    };

    CtpCmd* cmd = new CtpCmd;
    cmd->fn = fn;
    cmd->delayTick = delayTick;
    cmd->robotId = robotId;
    g_sm->ctpMgr()->runCmd(cmd);
}
