#include "pushservice.h"
#include "bfproxy.grpc.pb.h"
#include "ctp_utils.h"
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
    explicit PingCb(ProxyClient* proxyClient)
        : GrpcCb<BfPingData>()
        , proxyClient_(proxyClient)
    {
    }
    virtual ~PingCb() override {}

public:
    virtual void operator()() override;

private:
    ProxyClient* proxyClient_ = nullptr;
};

class ProxyClient {
public:
    ProxyClient(std::shared_ptr<grpc::Channel> channel, QString proxyId, const BfConnectReq& req)
        : stub_(BfProxyService::NewStub(channel))
        , proxyId_(proxyId)
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
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnTradeWillBegin(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnTradeWillBegin fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnTradeWillBegin(&pCb->context(), data, &cq_));
    }

    void OnGotContracts(const BfVoid& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnGotContracts(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnGotContracts fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnGotContracts(&pCb->context(), data, &cq_));
    }

    // ref: grpc\test\cpp\interop\interop_client.cc
    void OnPing(const BfPingData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfPingData reply;
        grpc::Status status = stub_->OnPing(&ctx, data, &reply);
        if (!status.ok()) {
            incPingFailCount();;
            BfError("(%s)->OnPing(%dms) fail(%d),code:%d,msg:%s",qPrintable(proxyId_),deadline_, pingFailCount(), status.error_code(), status.error_message().c_str());
            if (pingFailCount() > 3) {
                BfError("(%s)->OnPing fail too mang times,so kill it", qPrintable(proxyId_));
                QMetaObject::invokeMethod(g_sm->pushService(), "onProxyClose", Qt::QueuedConnection, Q_ARG(QString, proxyId_));
            }
            return;
        }
        resetPingFailCount();
        */

        auto pCb = new PingCb(this);
        pCb->setRpcPtrAndFinish(stub_->AsyncOnPing(&pCb->context(), data, &cq_));
    }

    void OnTick(const BfTickData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnTick(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnTick fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnTick(&pCb->context(), data, &cq_));
    }

    // 这个函数就别log了，会重入=
    void OnError(const BfErrorData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnError(&ctx, data, &reply);
        if (!status.ok()) {
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnError(&pCb->context(), data, &cq_));
    }

    // 这个函数就别log了，会重入=
    void OnLog(const BfLogData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnLog(&ctx, data, &reply);
        if (!status.ok()) {
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnLog(&pCb->context(), data, &cq_));
    }

    void OnTrade(const BfTradeData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnTrade(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnTrade fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnTrade(&pCb->context(), data, &cq_));
    }

    void OnOrder(const BfOrderData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnOrder(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnOrder fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnOrder(&pCb->context(), data, &cq_));
    }

    void OnPosition(const BfPositionData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnPosition(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnPosition fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnPosition(&pCb->context(), data, &cq_));
    }

    void OnAccount(const BfAccountData& data)
    {
        /*
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);

        BfVoid reply;
        grpc::Status status = stub_->OnAccount(&ctx, data, &reply);
        if (!status.ok()) {
            BfError("(%s)->OnAccount fail,code:%d,msg:%s", qPrintable(proxyId_), status.error_code(), status.error_message().c_str());
            return;
        }
        */
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
    QString proxyId() { return proxyId_; }

private:
    std::unique_ptr<BfProxyService::Stub> stub_;
    std::atomic_int32_t pingfail_count_ = 0;
    const int deadline_ = 500;
    QString proxyId_;
    BfConnectReq req_;

    // async client
    grpc::CompletionQueue cq_;
    QThread* cq_thread_ = nullptr;
};

void PingCb::operator()()
{
    if (!status_.ok()) {
        QString proxyId = proxyClient_->proxyId();
        proxyClient_->incPingFailCount();
        int failCount = proxyClient_->pingFailCount();
        int errorCode = status_.error_code();
        std::string errorMsg = status_.error_message();
        BfError("(%s)->OnPing(%dms) fail(%d),code:%d,msg:%s", qPrintable(proxyId), deadline_, failCount, errorCode, errorMsg.c_str());
        if (failCount > 3) {
            BfError("(%s)->OnPing fail too mang times,so kill it", qPrintable(proxyId));
            QMetaObject::invokeMethod(g_sm->pushService(), "onProxyClose", Qt::QueuedConnection, Q_ARG(QString, proxyId));
        }
        return;
    }
    proxyClient_->resetPingFailCount();
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

    // ctpmgr...
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &PushService::onTradeWillBegin);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotContracts, this, &PushService::onGotContracts);
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
    for (auto proxy : proxyClients_) {
        delete proxy;
    }
    proxyClients_.clear();
}

void PushService::onProxyConnect(const BfConnectReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString endpoint = QString().sprintf("%s:%d", req.proxyip().c_str(), req.proxyport());
    QString proxyId = req.proxyid().c_str();

    ProxyClient* proxyClient = new ProxyClient(grpc::CreateChannel(endpoint.toStdString(), grpc::InsecureChannelCredentials()),
        proxyId, req);

    if (proxyClients_.contains(proxyId)) {
        auto it = proxyClients_[proxyId];
        delete it;
        proxyClients_.remove(proxyId);
    }
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

void PushService::onGatewayClose()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        delete proxy;
    }
    proxyClients_.clear();
}

void PushService::onGotOrder(const BfOrderData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        if (proxy->tradehandler()) {
            proxy->OnOrder(data);
        }
    }
};

void PushService::onGotTrade(const BfTradeData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        if (proxy->tradehandler()) {
            proxy->OnTrade(data);
        }
    }
}

void PushService::onGotTick(void* curTick, void* preTick)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfTickData data;
    CtpUtils::translateTick(curTick, preTick, &data);
    for (auto proxy : proxyClients_) {
        if (proxy->tickHandler() && proxy->subscribled(data.symbol(), data.exchange())) {
            proxy->OnTick(data);
        }
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

void PushService::onGotContracts(QStringList ids, QStringList idsAll)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfVoid data;
    for (auto proxy : proxyClients_) {
        proxy->OnGotContracts(data);
    }
}

void PushService::onGotPosition(const BfPositionData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        if (proxy->tradehandler()) {
            proxy->OnPosition(data);
        }
    }
}

void PushService::onLog(QString when, QString msg)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    BfLogData data;
    data.set_when(when.toStdString());
    data.set_message(msg.toStdString());
    for (auto proxy : proxyClients_) {
        if (proxy->logHandler()) {
            proxy->OnLog(data);
        }
    }
}

void PushService::onGotAccount(const BfAccountData& data)
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    for (auto proxy : proxyClients_) {
        if (proxy->tradehandler()) {
            proxy->OnAccount(data);
        }
    }
}

void PushService::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    //google::protobuf::Arena arena;
    //BfPingData* data = google::protobuf::Arena::CreateMessage<BfPingData>(&arena);
    //data->set_message("bftrader");
    //for (auto proxy : proxyClients_) {
    //    proxy->OnPing(*data);
    //}
    BfPingData data;
    data.set_message("bftrader");
    for (auto proxy : proxyClients_) {
        proxy->OnPing(data);
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
        if (proxy->logHandler()) {
            proxy->OnError(data);
        }
    }
}
