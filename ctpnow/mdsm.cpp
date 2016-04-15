#include "mdsm.h"
#include "ThostFtdcMdApi.h"
#include "encode_utils.h"
#include "file_utils.h"
#include "logger.h"
#include "servicemgr.h"
#include <QDir>
#include <QMap>
#include <QTimer>

///////////
class MdSmSpi : public CThostFtdcMdSpi {
public:
    explicit MdSmSpi(MdSm* sm)
        : sm_(sm)
    {
    }

private:
    void OnFrontConnected() override
    {
        info(__FUNCTION__);
        emit sm()->statusChanged(MDSM_CONNECTED);
    }

    // 如果网络异常，会直接调用OnFrontDisconnected，需要重置状态数据=
    // 网络错误当再次恢复时候，会自动重连重新走OnFrontConnected
    void OnFrontDisconnected(int nReason) override
    {
        info(QString().sprintf("MdSmSpi::OnFrontDisconnected,nReason=0x%x",
            nReason));

        resetData();
        emit sm()->statusChanged(MDSM_DISCONNECTED);
    }

    // errorId=7，msg=CTP:还没有初始化=
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override
    {
        info(__FUNCTION__);
        if (bIsLast) {
            if (isErrorRsp(pRspInfo, nRequestID)) {
                emit sm()->statusChanged(MDSM_LOGINFAIL);
            } else {
                emit sm()->statusChanged(MDSM_LOGINED);
            }
        }
    }

    // logout在tdapi里面是有效的,mdapi无效不用处理=
    void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override {}

    void OnRspError(CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override
    {
        if (bIsLast) {
            info(__FUNCTION__);
            isErrorRsp(pRspInfo, nRequestID);
        }
    }

    // 订阅成功了也会调用,目前是不管啥都返回订阅成功=
    void OnRspSubMarketData(
        CThostFtdcSpecificInstrumentField* pSpecificInstrument,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override
    {
        if (!isErrorRsp(pRspInfo, nRequestID) && pSpecificInstrument) {
            QString iid = pSpecificInstrument->InstrumentID;
            got_ids_ << iid;
        }

        if (bIsLast && got_ids_.length()) {
            QString ids;
            for (auto id : got_ids_) {
                ids = ids + id + ";";
            }
            info(QString().sprintf("total sub ids:%d,%s", got_ids_.length(),
                ids.toUtf8().constData()));
        }
    }

    void OnRspUnSubMarketData(
        CThostFtdcSpecificInstrumentField* pSpecificInstrument,
        CThostFtdcRspInfoField* pRspInfo,
        int nRequestID,
        bool bIsLast) override {}

    void OnRtnDepthMarketData(
        CThostFtdcDepthMarketDataField* pDepthMarketData) override
    {
        // g_sm->dataPump()->putTick(pDepthMarketData);
    }

private:
    bool isErrorRsp(CThostFtdcRspInfoField* pRspInfo, int reqId)
    {
        if (pRspInfo && pRspInfo->ErrorID != 0) {
            info(QString().sprintf(
                "<==错误，reqid=%d,errorId=%d，msg=%s", reqId, pRspInfo->ErrorID,
                gbk2utf16(pRspInfo->ErrorMsg).toUtf8().constData()));
            return true;
        }
        return false;
    }

    MdSm* sm() { return sm_; }

    void resetData() { got_ids_.clear(); }

    void info(QString msg) { g_sm->logger()->info(msg); }

private:
    MdSm* sm_;
    QStringList got_ids_;
};

///////////
MdSm::MdSm(QObject* parent)
    : QObject(parent)
{
}

MdSm::~MdSm() {}

bool MdSm::init(QString userId,
    QString password,
    QString brokerId,
    QString frontMd,
    QString flowPathMd)
{
    userId_ = userId;
    password_ = password;
    brokerId_ = brokerId;
    frontMd_ = frontMd;
    flowPathMd_ = flowPathMd;

    // check
    if (userId_.length() == 0 || password_.length() == 0 || brokerId_.length() == 0 || frontMd_.length() == 0 || flowPathMd_.length() == 0) {
        return false;
    }
    return true;
}

void MdSm::start()
{
    info(__FUNCTION__);

    if (mdapi_ != nullptr) {
        qFatal("mdapi_!=nullptr");
        return;
    }

    QDir dir;
    dir.mkpath(flowPathMd_);
    mdapi_ = CThostFtdcMdApi::CreateFtdcMdApi(flowPathMd_.toStdString().c_str());
    mdspi_ = new MdSmSpi(this);
    mdapi_->RegisterSpi(mdspi_);
    mdapi_->RegisterFront((char*)qPrintable(frontMd_));
    mdapi_->Init();
}

void MdSm::stop()
{
    info(__FUNCTION__);

    if (mdapi_ == nullptr) {
        qFatal("mdapi_==nullptr");
        return;
    }

    mdapi_->RegisterSpi(nullptr);
    mdapi_->Release();
}

void MdSm::info(QString msg)
{
    g_sm->logger()->info(msg);
}

void MdSm::login(unsigned int delayTick, QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {

    });
}

void MdSm::subscrible(QStringList ids,
    unsigned int delayTick,
    QString robotId)
{
    info(__FUNCTION__);
    QTimer::singleShot(delayTick, this, [=] {

    });
}
