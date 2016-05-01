#include "pushservice.h"
#include "bfrobot.grpc.pb.h"
#include "grpc++/grpc++.h"
#include "servicemgr.h"
#include <QThread>
#include <atomic>

using namespace bftrader;
using namespace bftrader::bfrobot;

//
// RobotClient
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

class RobotClient;
class PingCb final : public GrpcCb<BfPingData> {
public:
    explicit PingCb(RobotClient* robotClient)
        : GrpcCb<BfPingData>()
        , robotClient_(robotClient)
    {
    }
    virtual ~PingCb() override {}

public:
    virtual void operator()() override;

private:
    RobotClient* robotClient_ = nullptr;
};

class RobotClient {
public:
    RobotClient(std::shared_ptr<grpc::Channel> channel, QString clientId, const BfConnectReq& req)
        : stub_(BfRobotService::NewStub(channel))
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
    ~RobotClient()
    {
        BfDebug(__FUNCTION__);
        cq_.Shutdown();
        cq_thread_->quit();
        cq_thread_->wait();
        delete cq_thread_;
        cq_thread_ = nullptr;
    }
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
    void OnInit(const BfVoid& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnInit(&pCb->context(), data, &cq_));
    }
    void OnStart(const BfVoid& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnStart(&pCb->context(), data, &cq_));
    }
    void OnStop(const BfVoid& data)
    {
        auto pCb = new GrpcCb<BfVoid>();
        pCb->setRpcPtrAndFinish(stub_->AsyncOnStop(&pCb->context(), data, &cq_));
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
    std::unique_ptr<BfRobotService::Stub> stub_;
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
        QString clientId = robotClient_->clientId();
        robotClient_->incPingFailCount();
        int failCount = robotClient_->pingFailCount();
        int errorCode = status_.error_code();
        std::string errorMsg = status_.error_message();
        BfError("(%s)->OnPing(%dms) fail(%d),code:%d,msg:%s", qPrintable(clientId), deadline_, failCount, errorCode, errorMsg.c_str());
        //if (failCount > 3) {
        //    BfError("(%s)->OnPing fail too mang times,so kill it", qPrintable(clientId));
        //    QMetaObject::invokeMethod(g_sm->pushService(), "disconnectRobot", Qt::QueuedConnection, Q_ARG(QString, clientId));
        //}
        return;
    }
    robotClient_->resetPingFailCount();
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
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}
