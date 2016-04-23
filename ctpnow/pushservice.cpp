#include "pushservice.h"
#include "bfproxy.grpc.pb.h"
#include "ctp_utils.h"
#include "logger.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfproxy;

//
// ProxyClient
//
class ProxyClient {
public:
    ProxyClient(std::shared_ptr<grpc::Channel> channel, QString proxyId)
        : stub_(BfProxyService::NewStub(channel))
        , proxyId_(proxyId)
    {
        BfDebug(__FUNCTION__);
    }
    ~ProxyClient() {}

    void OnTradeWillBegin(const BfVoid& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnTradeWillBegin(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnTradeWillBegin fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    // ref: grpc\test\cpp\interop\interop_client.cc
    void OnPing(const BfPingData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfPingData reply;
        grpc::Status status = stub_->OnPing(&ctx, data, &reply);
        if (!status.ok()) {
            pingfail_count_++;
            BfError("(%s)->OnPing fail(%d),code:%d,msg:%s", qPrintable(proxyId_), pingfail_count_, status.error_code(), status.error_message().c_str());
            if (pingfail_count_ > 3) {
                BfError("(%s)->OnPing fail too long,so kill it", qPrintable(proxyId_));
                QMetaObject::invokeMethod(g_sm->pushService(), "onProxyClose", Qt::QueuedConnection, Q_ARG(QString, proxyId_));
            }
            return;
        }
        pingfail_count_ = 0;

        if (reply.message() != data.message()) {
            BfError("(%s)->OnPing fail,ping:%s,pong:%s", qPrintable(proxyId_), data.message().c_str(), reply.message().c_str());
            return;
        }
    }

    void OnTick(const BfTickData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnTick(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnTick fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    void OnError(const BfErrorData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnError(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnError fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    void OnLog(const BfLogData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnLog(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnLog fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    void OnTrade(const BfTradeData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnTrade(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnTrade fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    void OnOrder(const BfOrderData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnOrder(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnOrder fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    void OnPosition(const BfPositionData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnPosition(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnPosition fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

    void OnAccount(const BfAccountData& data)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ;
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnAccount(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnAccount fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
    }

private:
    std::unique_ptr<BfProxyService::Stub> stub_;
    int pingfail_count_ = 0;
    const int deadline_ = 100;
    QString proxyId_;
};

//
// PushService
//

PushService::PushService(QObject* parent)
    : QObject(parent)
{
}

void PushService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &PushService::onPing);
    this->pingTimer_->start();

    // ctpmgr...
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &PushService::onTradeWillBegin);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTick, this, &PushService::onGotTick);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotOrder, this, &PushService::onGotOrder);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTrade, this, &PushService::onGotTrade);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotPosition, this, &PushService::onGotPosition);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotAccount, this, &PushService::onGotAccount);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotCtpError, this, &PushService::onCtpError);
    QObject::connect(g_sm->logger(), &Logger::gotError, this, &PushService::onLog);
    QObject::connect(g_sm->logger(), &Logger::gotInfo, this, &PushService::onLog);
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // close timer
    this->pingTimer_->stop();
    delete this->pingTimer_;
    this->pingTimer_ = nullptr;

    // delete all proxyclient
    for (int i = 0; i < proxyClients_.size(); i++) {
        auto proxyClient = proxyClients_.values().at(i);
        delete proxyClient;
    }
    proxyClients_.clear();
}

void PushService::onProxyConnect(QString proxyId, QString proxyIp, qint32 proxyPort)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString endpoint = QString().sprintf("%s:%d", proxyIp.toStdString().c_str(), proxyPort);

    ProxyClient* proxyClient = new ProxyClient(grpc::CreateChannel(endpoint.toStdString(), grpc::InsecureChannelCredentials()),
        proxyId);

    proxyClients_[proxyId] = proxyClient;
}

void PushService::onProxyClose(QString proxyId)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (proxyClients_.contains(proxyId)) {
        BfDebug("delete proxyclient:%s", qPrintable(proxyId));
        ProxyClient* proxyClient = proxyClients_[proxyId];
        delete proxyClient;
        proxyClients_.remove(proxyId);
    }
}

void PushService::onGotOrder(const BfOrderData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        proxy->OnOrder(data);
    }
};

void PushService::onGotTrade(const BfTradeData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        proxy->OnTrade(data);
    }
}

void PushService::onGotTick(void* curTick, void* preTick)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfTickData data;
    CtpUtils::translateTick(curTick, preTick, &data);
    for (auto proxy : proxyClients_) {
        proxy->OnTick(data);
    }
}

void PushService::onTradeWillBegin()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfVoid data;
    for (auto proxy : proxyClients_) {
        proxy->OnTradeWillBegin(data);
    }
}

void PushService::onGotPosition(const BfPositionData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        proxy->OnPosition(data);
    }
}

void PushService::onLog(QString when, QString msg)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfLogData data;
    data.set_when(when.toStdString());
    data.set_message(msg.toStdString());
    for (auto proxy : proxyClients_) {
        proxy->OnLog(data);
    }
}

void PushService::onGotAccount(const BfAccountData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        proxy->OnAccount(data);
    }
}

void PushService::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    google::protobuf::Arena arena;
    BfPingData* data = google::protobuf::Arena::CreateMessage<BfPingData>(&arena);
    data->set_message("bftrader");
    for (auto proxy : proxyClients_) {
        proxy->OnPing(*data);
    }
}

void PushService::onCtpError(int code, QString msg, QString msgEx)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfErrorData data;
    data.set_errorid(code);
    data.set_errormsg(msg.toStdString());
    data.set_additionalinfo(msgEx.toStdString());
    for (auto proxy : proxyClients_) {
        proxy->OnError(data);
    }
}
