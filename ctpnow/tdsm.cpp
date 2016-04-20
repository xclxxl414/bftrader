#include "tdsm.h"
#include "ThostFtdcTraderApi.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "logger.h"
#include "servicemgr.h"
#include <QDir>

///////////
namespace {
char translatePriceType(BfPriceType priceType)
{
    switch (priceType) {
    case PRICETYPE_LIMITPRICE:
        return THOST_FTDC_OPT_LimitPrice;
    case PRICETYPE_MARKETPRICE:
        return THOST_FTDC_OPT_AnyPrice;
    default:
        qFatal("invalid BfPriceType");
    }
    return '0';
}

char translateOffset(BfOffset offset)
{
    switch (offset) {
    case OFFSET_OPEN:
        return THOST_FTDC_OF_Open;
    case OFFSET_CLOSE:
        return THOST_FTDC_OF_Close;
    case OFFSET_CLOSETODAY:
        return THOST_FTDC_OF_CloseToday;
    case OFFSET_CLOSEYESTERDAY:
        return THOST_FTDC_OF_CloseYesterday;
    default:
        qFatal("invalid offset");
    }
    return '0';
}

char translateDirection(BfDirection direction)
{
    switch (direction) {
    case DIRECTION_LONG:
        return THOST_FTDC_D_Buy;
    case DIRECTION_SHORT:
        return THOST_FTDC_D_Sell;
    default:
        qFatal("invalid direction");
    }
    return '0';
}

BfPriceType translatePriceType(char priceType)
{
    switch (priceType) {
    case THOST_FTDC_OPT_LimitPrice:
        return PRICETYPE_LIMITPRICE;
    case THOST_FTDC_OPT_AnyPrice:
        return PRICETYPE_MARKETPRICE;
    default:
        qFatal("invalid BfPriceType");
    }
    return PRICETYPE_UNKONWN;
}

BfOffset translateOffset(char offset)
{
    switch (offset) {
    case THOST_FTDC_OF_Open:
        return OFFSET_OPEN;
    case THOST_FTDC_OF_Close:
        return OFFSET_CLOSE;
    case THOST_FTDC_OF_CloseToday:
        return OFFSET_CLOSETODAY;
    case THOST_FTDC_OF_CloseYesterday:
        return OFFSET_CLOSEYESTERDAY;
    default:
        g_sm->logger()->info("invalid offset");
    }
    return OFFSET_UNKNOWN;
}

// 方向类型映射=
BfDirection translateDirection(char direction)
{
    switch (direction) {
    case THOST_FTDC_D_Buy:
        return DIRECTION_LONG;
    case THOST_FTDC_D_Sell:
        return DIRECTION_SHORT;
    default:
        g_sm->logger()->info("invalid direction");
    }
    return DIRECTION_UNKNOWN;
}

// 持仓类型映射=
BfDirection translatePosDirection(char direction)
{
    switch (direction) {
    case THOST_FTDC_PD_Net:
        return DIRECTION_NET;
    case THOST_FTDC_PD_Long:
        return DIRECTION_LONG;
    case THOST_FTDC_PD_Short:
        return DIRECTION_SHORT;
    default:
        g_sm->logger()->info("invalid direction");
    }
    return DIRECTION_UNKNOWN;
}

BfStatus translateStatus(char status)
{
    switch (status) {
    case THOST_FTDC_OST_AllTraded:
        return STATUS_ALLTRADED;
    case THOST_FTDC_OST_PartTradedQueueing:
        return STATUS_PARTTRADED;
    case THOST_FTDC_OST_NoTradeQueueing:
        return STATUS_NOTTRADED;
    case THOST_FTDC_OST_Canceled:
        return STATUS_CANCELLED;
    default:
        g_sm->logger()->info("invalid status");
    }
    return STATUS_UNKNOWN;
}
}

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

    // 请求查询投资者持仓响应=
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (!isErrorRsp(pRspInfo, nRequestID) && pInvestorPosition) {
            BfPositionData pos;

            // 保存代码=
            pos.set_symbol(pInvestorPosition->InstrumentID);
            pos.set_direction(translatePosDirection(pInvestorPosition->PosiDirection));

            // 冻结=
            if (pos.direction() == DIRECTION_LONG || pos.direction() == DIRECTION_NET) {
                pos.set_frozen(pInvestorPosition->LongFrozen);
            } else if (pos.direction() == DIRECTION_SHORT) {
                pos.set_frozen(pInvestorPosition->ShortFrozen);
            }

            // 持仓量=
            pos.set_position(pInvestorPosition->Position);

            // 均价=
            if (pos.position() > 0) {
                pos.set_price(pInvestorPosition->PositionCost / pos.position());
            }

            emit g_sm->ctpMgr()->gotPosition(pos);
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

    // 报单录入请求响应=
    // 发单错误（柜台）=
    void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                int orderId = QString(pInputOrder->OrderRef).toInt();
                info(QString().sprintf("OnRspOrderInsert: orderId = %d", orderId));
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
            if (isErrorRsp(pRspInfo, nRequestID)) {
                int orderId = QString(pInputOrderAction->OrderRef).toInt();
                info(QString().sprintf("OnRspOrderAction: orderId = %d", orderId));
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
            if (isErrorRsp(pRspInfo, 0)) {
                int orderId = QString(pInputOrder->OrderRef).toInt();
                info(QString().sprintf("OnErrRtnOrderInsert: orderId = %d", orderId));
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
            if (isErrorRsp(pRspInfo, 0)) {
                int orderId = QString(pOrderAction->OrderRef).toInt();
                info(QString().sprintf("OnErrRtnOrderAction: orderId = %d", orderId));
                return;
            } else {
            }
        }
    }

    // 报单通知=
    virtual void OnRtnOrder(CThostFtdcOrderField* pOrder)
    {
        info(__FUNCTION__);

        int orderId = QString(pOrder->OrderRef).toInt();
        orderRef_ = qMax(orderId, orderRef_);

        info(QString().sprintf("OnRtnOrder: orderId = %d", orderId));

        BfOrderData order;
        // 保存代码和报单号=
        order.set_exchange(pOrder->ExchangeID);
        order.set_symbol(pOrder->InstrumentID);
        order.set_orderid(QString::number(orderId).toStdString());
        order.set_direction(translateDirection(pOrder->Direction)); //方向=
        order.set_offset(translateOffset(pOrder->CombOffsetFlag[0])); //开平=
        order.set_status(translateStatus(pOrder->OrderStatus)); //状态=

        //价格、报单量等数值=
        order.set_price(pOrder->LimitPrice);
        order.set_totalvolume(pOrder->VolumeTotalOriginal);
        order.set_tradedvolume(pOrder->VolumeTraded);
        order.set_ordertime(pOrder->InsertTime);
        order.set_canceltime(pOrder->CancelTime);

        emit g_sm->ctpMgr()->gotOrder(order);
    }

    // 成交通知=
    virtual void OnRtnTrade(CThostFtdcTradeField* pTrade)
    {
        info(__FUNCTION__);

        int orderId = QString(pTrade->OrderRef).toInt();
        int tradeId = QString(pTrade->TradeID).toInt();

        BfTradeData trade;
        // 保存代码和报单号=
        trade.set_exchange(pTrade->ExchangeID);
        trade.set_symbol(pTrade->InstrumentID);
        trade.set_orderid(QString::number(orderId).toStdString());
        trade.set_tradeid(QString::number(tradeId).toStdString());
        trade.set_direction(translateDirection(pTrade->Direction)); //方向=
        trade.set_offset(translateOffset(pTrade->OffsetFlag)); //开平=

        //价格、报单量等数值=
        trade.set_price(pTrade->Price);
        trade.set_volume(pTrade->Volume);
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

void TdSm::sendOrder(unsigned int delayTick, QString robotId, const BfOrderReq& orderReq)
{
    info(__FUNCTION__);

    std::function<int(int, QString)> fn = [=](int reqId, QString robotId) -> int {
        CThostFtdcInputOrderField req;
        memset(&req, 0, sizeof(req));
        int orderRef = tdspi_->getOrderRef();
        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.InvestorID, userId_.toStdString().c_str(), sizeof(req.InvestorID) - 1);
        strncpy(req.UserID, userId_.toStdString().c_str(), sizeof(req.UserID) - 1);
        strncpy(req.OrderRef, QString::number(orderRef).toStdString().c_str(), sizeof(req.OrderRef) - 1);
        strncpy(req.InstrumentID, orderReq.symbol().c_str(), sizeof(req.InstrumentID) - 1);

        req.Direction = translateDirection(orderReq.direction());
        req.CombOffsetFlag[0] = translateOffset(orderReq.offset());
        req.OrderPriceType = translatePriceType(orderReq.pricetype());
        req.LimitPrice = orderReq.price();
        req.VolumeTotalOriginal = orderReq.volume();

        req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation; // 投机单=
        req.ContingentCondition = THOST_FTDC_CC_Immediately; // 立即发单=
        req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose; // 非强平=
        req.IsAutoSuspend = 0; // 非自动挂起=
        req.TimeCondition = THOST_FTDC_TC_GFD; // 今日有效=
        req.VolumeCondition = THOST_FTDC_VC_AV; // 任意成交量=
        req.MinVolume = 1; // 最小成交量为1=

        int result = tdapi_->ReqOrderInsert(&req, reqId);
        info(QString().sprintf("CmdTdReqOrderInsert,orderRef=%d,reqId=%d,result=%d", orderRef, reqId, result));
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
