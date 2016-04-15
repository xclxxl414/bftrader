#include "TdSm.h"
#include "ThostFtdcTraderApi.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "logger.h"
#include "servicemgr.h"
#include <QDir>
#include <QTimer>
#include <leveldb/db.h>

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

        resetData();
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
                        //......
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
            emit sm()->gotInstruments(ids_);
        }
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
        ids_.clear();
        idPrefixList_.clear();
    }

    void info(QString msg)
    {
        g_sm->logger()->info(msg);
    }

private:
    TdSm* sm_;
    QStringList ids_;
    QStringList idPrefixList_;
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

    tdapi_->RegisterSpi(nullptr);
    tdapi_->Release();

    tdapi_ = nullptr;
    delete tdspi_;
    tdspi_ = nullptr;
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

void TdSm::login(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {
        CThostFtdcReqUserLoginField req;
        memset(&req, 0, sizeof(req));
        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.UserID, userId_.toStdString().c_str(), sizeof(req.UserID) - 1);
        strncpy(req.Password, password_.toStdString().c_str(), sizeof(req.Password) - 1);
        int result = tdapi_->ReqUserLogin(&req, ++reqId_);
        info(QString().sprintf("CmdTdLogin,reqId=%d,result=%d", reqId_, result));
        //  被流控，一秒后重来=
        if (result == -3) {
            login(RESEND_AFTER_MSEC, robotId);
        } else {
            //发单成功，发信号，<reqId,robotId>，便于上层跟踪=
            emit requestSent(reqId_, robotId);
        }
    });
}

//目前，通过 ReqUserLogout 登出系统的话，会先将现有的连接断开，再重新建立一个新的连接(CTPSDK)
// logout之后会有一个disconnect/connect...先disableautologin
void TdSm::logout(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {
        CThostFtdcUserLogoutField req;
        memset(&req, 0, sizeof(req));
        strncpy(req.BrokerID, brokerId_.toStdString().c_str(), sizeof(req.BrokerID) - 1);
        strncpy(req.UserID, userId_.toStdString().c_str(), sizeof(req.UserID) - 1);
        int result = tdapi_->ReqUserLogout(&req, ++reqId_);
        info(QString().sprintf("CmdTdLogout,reqId=%d,result=%d", reqId_, result));
        //  被流控，一秒后重来=
        if (result == -3) {
            logout(RESEND_AFTER_MSEC, robotId);
        } else {
            //发单成功，发信号，<reqId,robotId>，便于上层跟踪=
            emit requestSent(reqId_, robotId);
        }
    });
}

void TdSm::queryInstrument(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {
        CThostFtdcQryInstrumentField req;
        memset(&req, 0, sizeof(req));
        int result = tdapi_->ReqQryInstrument(&req, ++reqId_);
        info(QString().sprintf("CmdTdQueryInstrument,reqId=%d,result=%d", reqId_, result));
        //  被流控，一秒后重来=
        if (result == -3) {
            queryInstrument(RESEND_AFTER_MSEC, robotId);
        } else {
            //发单成功，发信号，<reqId,robotId>，便于上层跟踪=
            emit requestSent(reqId_, robotId);
        }
    });
}
