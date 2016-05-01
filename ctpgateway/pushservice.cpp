#include "pushservice.h"
#include "bfproxy.grpc.pb.h"
#include "ctputils.h"
#include "logger.h"
#include "servicemgr.h"
#include <QThread>
#include <atomic>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfproxy;

//
// ProxyClient，实现异步客户端，不需要等客户端的应答就直接返回，避免一个客户端堵住其他的=
//

class IGrpcCb {
public:
    explicit IGrpcCb()
    {
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        context_.set_deadline(deadline);
    }
    virtual ~IGrpcCb()
    {
    }

    grpc::ClientContext& context() { return context_; }
    grpc::Status& status() { return status_; }

public:
    virtual void operator()() {}

protected:
    grpc::ClientContext context_;
    grpc::Status status_;
    const int deadline_ = 500;
};

template <class Resp>
class GrpcCb : public IGrpcCb {
public:
    explicit GrpcCb()
        : IGrpcCb()
    {
    }
    virtual ~GrpcCb() override {}

public:
    typedef std::unique_ptr<grpc::ClientAsyncResponseReader<Resp> > RpcPtr;

public:
    Resp& getResp() { return resp_; }
    void setRpcPtrAndFinish(RpcPtr rpc)
    {
        rpc_.swap(rpc);
        rpc_->Finish(&resp_, &status_, (void*)this);
    }

public:
    virtual void operator()() override
    {
    }

private:
    RpcPtr rpc_;
    Resp resp_;
};

class ProxyClient;
class PingCb final : public GrpcCb<BfPingData> {
public:
    explicit PingCb(ProxyClient* client)
        : GrpcCb<BfPingData>()
        , client_(client)
    {
    }
    virtual ~PingCb() override {}

public:
    virtual void operator()() override;

private:
    ProxyClient* client_ = nullptr;
};

class ProxyClient {
public:
    ProxyClient(std::shared_ptr<grpc::Channel> channel, QString clientId, const BfConnectReq& req)
        : stub_(BfProxyService::NewStub(channel))
        , clientId_(clientId)
        , req_(req)
    {
        BfDebug(__FUNCTION__);
        cq_thread_ = new QThread();
        std::function<void(void)> fn = [=]() {
            for (;;) {
                void* pTag;
                bool ok = false;
                bool result = this->cq_.Next(&pTag, &ok);
                if (result) {
                    std::unique_ptr<IGrpcCb> pCb(static_cast<IGrpcCb*>(pTag));
                    // run callback
                    (*pCb)();
                } else {
                    // shutdown
                    BfDebug("cq_thread shutdown");
                    break;
                }
            }
        };
        QObject::connect(cq_thread_, &QThread::started, fn);
        cq_thread_->start();
    }
    ~ProxyClient()
    {
        BfDebug(__FUNCTION__);
        cq_.Shutdown();
        cq_thread_->quit();
        cq_thread_->wait();
        delete cq_thread_;
        cq_thread_ = nullptr;
    }

    void OnTradeWillBegin(const BfVoid& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnTradeWillBegin(&pCb->context(), data, &cq_));
    }

    void OnGotContracts(const BfVoid& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnGotContracts(&pCb->context(), data, &cq_));
    }

    // ref: grpc\test\cpp\interop\interop_client.cc
    void OnPing(const BfPingData& data)
    {
        auto pCb = new PingCb(this);
        pCb->setRpcPtrAndFinish(stub_->AsyncOnPing(&pCb->context(), data, &cq_));
    }

    void OnTick(const BfTickData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnTick(&pCb->context(), data, &cq_));
    }

    // 这个函数就别log了，会重入=
    void OnError(const BfErrorData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnError(&pCb->context(), data, &cq_));
    }

    // 这个函数就别log了，会重入=
    void OnLog(const BfLogData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnLog(&pCb->context(), data, &cq_));
    }

    void OnTrade(const BfTradeData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnTrade(&pCb->context(), data, &cq_));
    }

    void OnOrder(const BfOrderData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnOrder(&pCb->context(), data, &cq_));
    }

    void OnPosition(const BfPositionData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnPosition(&pCb->context(), data, &cq_));
    }

    void OnAccount(const BfAccountData& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnAccount(&pCb->context(), data, &cq_));
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
    void incPingFailCount() { pingfail_count_++; }
    int pingFailCount() { return pingfail_count_; }
    void resetPingFailCount() { pingfail_count_ = 0; }
    QString clientId() { return clientId_; }

private:
    std::unique_ptr<BfProxyService::Stub> stub_;
    std::atomic_int32_t pingfail_count_ = 0;
    const int deadline_ = 500;
    QString clientId_;
    BfConnectReq req_;

    // async client
    grpc::CompletionQueue cq_;
    QThread* cq_thread_ = nullptr;
};

void PingCb::operator()()
{
    if (!status_.ok()) {
        QString clientId = client_->clientId();
        client_->incPingFailCount();
        int failCount = client_->pingFailCount();
        int errorCode = status_.error_code();
        std::string errorMsg = status_.error_message();
        BfError("(%s)->OnPing(%dms) fail(%d),code:%d,msg:%s", qPrintable(clientId), deadline_, failCount, errorCode, errorMsg.c_str());
        //if (failCount > 3) {
        //    BfError("(%s)->OnPing fail too mang times,so kill it", qPrintable(clientId));
        //    QMetaObject::invokeMethod(g_sm->pushService(), "disconnectProxy", Qt::QueuedConnection, Q_ARG(QString, clientId));
        //}
        return;
    }
    client_->resetPingFailCount();
}

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

    // delete all proxyclient
    for (auto client : clients_) {
        delete client;
    }
    clients_.clear();
}

void PushService::connectProxy(const BfConnectReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString endpoint = QString().sprintf("%s:%d", req.clientip().c_str(), req.clientport());
    QString clientId = req.clientid().c_str();

    ProxyClient* client = new ProxyClient(grpc::CreateChannel(endpoint.toStdString(), grpc::InsecureChannelCredentials()),
        clientId, req);

    if (clients_.contains(clientId)) {
        auto it = clients_[clientId];
        delete it;
        clients_.remove(clientId);
    }
    clients_[clientId] = client;
}

void PushService::disconnectProxy(QString clientId)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if (clients_.contains(clientId)) {
        BfDebug("delete proxyclient:%s", qPrintable(clientId));
        ProxyClient* client = clients_[clientId];
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

    BfVoid data;
    for (auto client : clients_) {
        client->OnTradeWillBegin(data);
    }
}

void PushService::onGotContracts(QStringList ids, QStringList idsAll)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfVoid data;
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
