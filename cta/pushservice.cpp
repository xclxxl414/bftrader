#include "pushservice.h"
#include "bfcta.grpc.pb.h"
#include "dbservice.h"
#include "grpc++/grpc++.h"
#include "safequeue.h"
#include "servicemgr.h"
#include <QThread>
#include <atomic>

using namespace bfcta;

class CtaClient {
public:
    CtaClient(SafeQueue<google::protobuf::Any>* queue, const BfConnectPushReq& req)
        : queue_(queue)
        , req_(req)
    {
        BfDebug("(%s)->CtaClient", qPrintable(clientId()));
    }
    ~CtaClient()
    {
        BfDebug("(%s)->~CtaClient", qPrintable(clientId()));
        // NOTE(hege):关闭队列=
        shutdown();
    }

    void OnPing(const BfPingData& data)
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
    BfConnectPushReq req_;
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

    // gatewaymgr
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotOrder, this, &PushService::onGotOrder);
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotTrade, this, &PushService::onGotTrade);
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // close timer
    this->pingTimer_->stop();
    delete this->pingTimer_;
    this->pingTimer_ = nullptr;

    // delete all ctaclient
    for (auto client : clients_) {
        delete client;
    }
    clients_.clear();
}

void PushService::connectClient(const BfConnectPushReq& req, void* queue)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString clientId = req.clientid().c_str();
    BfDebug("(%s)->connectClient", qPrintable(clientId));

    auto client = new CtaClient((SafeQueue<google::protobuf::Any>*)queue, req);
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

void PushService::onCtaClosed()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto client : clients_) {
        delete client;
    }
    clients_.clear();
}

void PushService::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfPingData data;
    data.set_message("cta");
    for (auto client : clients_) {
        client->OnPing(data);
    }
}

void PushService::onGotTrade(QString gatewayId, const BfTradeData& data)
{
    for (auto client : clients_) {
        if (client->tradehandler()) {
            client->OnTrade(data);
        }
        return;
    }
}

void PushService::onGotOrder(QString gatewayId, const BfOrderData& data)
{
    for (auto client : clients_) {
        if (client->tradehandler()) {
            client->OnOrder(data);
        }
        return;
    }
}
