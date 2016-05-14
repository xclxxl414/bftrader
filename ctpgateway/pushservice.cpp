#include "pushservice.h"
#include "bfgateway.grpc.pb.h"
#include "ctputils.h"
#include "logger.h"
#include "safequeue.h"
#include "servicemgr.h"
#include <QThread>
#include <atomic>
#include <grpc++/grpc++.h>

using namespace bftrader;

class GatewayClient {
public:
    GatewayClient(SafeQueue<google::protobuf::Any>* queue, QString gatewayId, const BfConnectReq& req)
        : queue_(queue)
        , gatewayId_(gatewayId)
        , req_(req)
    {
        BfDebug("(%s)->GatewayClient", qPrintable(clientId()));
    }
    ~GatewayClient()
    {
        BfDebug("(%s)->~GatewayClient", qPrintable(clientId()));
        // NOTE(hege):关闭队列=
        shutdown();
    }

    void OnTradeWillBegin(const BfNotificationData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnGotContracts(const BfNotificationData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnPing(const BfPingData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnTick(const BfTickData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    // 这个函数就别log了，会重入=
    void OnError(const BfErrorData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    // 这个函数就别log了，会重入=
    void OnLog(const BfLogData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnTrade(const BfTradeData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnOrder(const BfOrderData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnPosition(const BfPositionData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

    void OnAccount(const BfAccountData& data)
    {
        auto any = new google::protobuf::Any();
        any->PackFrom(data);
        queue_->enqueue(any);
    }

public:
    bool logHandler() { return req_.loghandler(); }
    bool tickHandler() { return req_.tickhandler(); }
    bool tradehandler() { return req_.tradehandler(); }
    bool subscribled(const std::string& symbol, const std::string& exchange)
    {
        if (req_.symbol() == "*") {
            return true;
        }
        if (symbol == req_.symbol()) {
            return true;
        }
        return false;
    }
    QString gatewayId() { return gatewayId_; }
    QString clientId() { return req_.clientid().c_str(); }

private:
    //NOTE(hege):由于跨线程，这里shutdown后等1秒钟=
    void shutdown()
    {
        if (queue_) {
            queue_->shutdown();
            Sleep(1000);
            delete queue_;
            queue_ = nullptr;
        }
    }

private:
    SafeQueue<google::protobuf::Any>* queue_ = nullptr;
    QString gatewayId_;
    BfConnectReq req_;
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
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &PushService::onPing);
    this->pingTimer_->start();

    // gatewaymgr...
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::tradeWillBegin, this, &PushService::onTradeWillBegin);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotContracts, this, &PushService::onGotContracts);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotTick, this, &PushService::onGotTick);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotOrder, this, &PushService::onGotOrder);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotTrade, this, &PushService::onGotTrade);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotPosition, this, &PushService::onGotPosition);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotAccount, this, &PushService::onGotAccount);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotGatewayError, this, &PushService::onGatewayError);
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

    // delete all gatewayclient
    for (auto client : clients_) {
        delete client;
    }
    clients_.clear();
}

void PushService::connectClient(QString gatewayId, const BfConnectReq& req, void* queue)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString clientId = req.clientid().c_str();

    BfDebug("(%s)->connectClient", qPrintable(clientId));
    auto client = new GatewayClient((SafeQueue<google::protobuf::Any>*)queue, gatewayId, req);
    if (clients_.contains(clientId)) {
        auto it = clients_[clientId];
        delete it;
        clients_.remove(clientId);
    }
    clients_[clientId] = client;
}

void PushService::disconnectClient(QString clientId)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (clients_.contains(clientId)) {
        BfDebug("(%s)->disconnectClient", qPrintable(clientId));
        auto client = clients_[clientId];
        delete client;
        clients_.remove(clientId);
    }
}

void PushService::onGatewayClosed()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto client : clients_) {
        delete client;
    }
    clients_.clear();
}

void PushService::onGotOrder(const BfOrderData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto client : clients_) {
        if (client->tradehandler()) {
            client->OnOrder(data);
        }
    }
};

void PushService::onGotTrade(const BfTradeData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto client : clients_) {
        if (client->tradehandler()) {
            client->OnTrade(data);
        }
    }
}

void PushService::onGotTick(void* curTick, void* preTick)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfTickData data;
    CtpUtils::translateTick(curTick, preTick, &data);

    // tick里面的exchange不一定有=
    QString exchange = data.exchange().c_str();
    if (exchange.trimmed().length() == 0) {
        void* contract = g_sm->gatewayMgr()->getContract(data.symbol().c_str());
        exchange = CtpUtils::getExchangeFromContract(contract);
        data.set_exchange(exchange.toStdString());
    }

    for (auto client : clients_) {
        if (client->tickHandler() && client->subscribled(data.symbol(), data.exchange())) {
            client->OnTick(data);
        }
    }
}

void PushService::onTradeWillBegin()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfNotificationData data;
    data.set_code(NOTIFICATION_TRADEWILLBEGIN);
    for (auto client : clients_) {
        client->OnTradeWillBegin(data);
    }
}

void PushService::onGotContracts(QStringList ids, QStringList idsAll)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfNotificationData data;
    data.set_code(NOTIFICATION_GOTCONTRACTS);
    for (auto client : clients_) {
        client->OnGotContracts(data);
    }
}

void PushService::onGotPosition(const BfPositionData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto client : clients_) {
        if (client->tradehandler()) {
            client->OnPosition(data);
        }
    }
}

void PushService::onLog(QString when, QString msg)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfLogData data;
    data.set_when(when.toStdString());
    data.set_message(msg.toStdString());
    for (auto client : clients_) {
        if (client->logHandler()) {
            client->OnLog(data);
        }
    }
}

void PushService::onGotAccount(const BfAccountData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto client : clients_) {
        if (client->tradehandler()) {
            client->OnAccount(data);
        }
    }
}

void PushService::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfPingData data;
    data.set_message("ctpgateway");
    for (auto client : clients_) {
        client->OnPing(data);
    }
}

void PushService::onGatewayError(int code, QString msg, QString msgEx)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfErrorData data;
    data.set_code(code);
    data.set_message(msg.toStdString());
    data.set_messageex(msgEx.toStdString());
    for (auto client : clients_) {
        if (client->logHandler()) {
            client->OnError(data);
        }
    }
}
