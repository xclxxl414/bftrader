#include "gatewaymgr.h"
#include "bfgateway.grpc.pb.h"
#include "servicemgr.h"
#include <QThread>
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfgateway;

//
// GatewayClient
//
class GatewayClient {
public:
    GatewayClient(std::shared_ptr<grpc::Channel> channel, QString gatewayId, const BfConnectReq& req)
        : stub_(BfGatewayService::NewStub(channel))
        , gatewayId_(gatewayId)
        , req_(req)
        , channel_(channel.get())
    {
        BfDebug(__FUNCTION__);
    }
    ~GatewayClient()
    {
        BfDebug(__FUNCTION__);
        freeReaderThread();
    }
    bool ready()
    {
        if (GRPC_CHANNEL_READY == channel_->GetState(true)) {
            return true;
        }
        return false;
    }

    bool connected()
    {
        if (reader_thread_) {
            return true;
        }
        return false;
    }

    void freeReaderThread()
    {
        if (reader_thread_) {
            // disconnect
            BfVoid req, resp;
            this->Disconnect(req, resp);

            // free
            reader_thread_->quit();
            reader_thread_->wait();
            delete reader_thread_;
            reader_thread_ = nullptr;
        }
    }

public:
    // ref: grpc\test\cpp\interop\interop_client.cc
    void Ping(const BfPingData& req, BfPingData& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->Ping(&ctx, req, &resp);
        if (!status.ok()) {
            pingfail_count_++;
            BfError("(%s)->Ping fail(%d),code:%d,msg:%s", qPrintable(gatewayId_), pingfail_count_, status.error_code(), status.error_message().c_str());
            //if (pingfail_count_ > 3) {
            //    BfError("(%s)->Ping fail too long,so kill it", qPrintable(gatewayId_));
            //    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "disconnectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId_));
            //}
            return;
        }
        pingfail_count_ = 0;

        if (req.message() != resp.message()) {
            BfError("(%s)->Ping fail,ping:%s,pong:%s", qPrintable(gatewayId_), req.message().c_str(), resp.message().c_str());
            return;
        }
    }

    // NOTE(hege):要么正常的disconnect让服务端关闭，要么网络异常导致read失败=
    void Connect()
    {
        BfInfo("(%s)->Connect now!", qPrintable(gatewayId_));
        BfConnectReq req = req_;
        reader_thread_ = new QThread();
        std::function<void(void)> fn = [=]() {
            grpc::ClientContext ctx;
            std::unique_ptr< ::grpc::ClientReader< ::google::protobuf::Any> > reader = stub_->Connect(&ctx, req);
            for (;;) {
                google::protobuf::Any any;
                bool ok = reader->Read(&any);
                if (ok) {
                    dispatchPush(any);
                } else {
                    // shutdown
                    BfDebug("stream shutdown");
                    grpc::Status status = reader->Finish();
                    if (!status.ok()) {
                        BfError("(%s)->Connect,code:%d,msg:%s", qPrintable(this->gatewayId_), status.error_code(), status.error_message().c_str());
                    }
                    // freeReaderThread
                    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "onGatewayDisconnected", Qt::QueuedConnection, Q_ARG(QString, this->gatewayId_));
                    break;
                }
            }
        };
        QObject::connect(reader_thread_, &QThread::started, fn);
        reader_thread_->start();
    }

    void Disconnect(const BfVoid& req, BfVoid& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->Disconnect(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->Disconnect,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

    void GetContract(const BfGetContractReq& req, BfContractData& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->GetContract(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->GetContract,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

    void SendOrder(const BfSendOrderReq& req, BfSendOrderResp& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->SendOrder(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->SendOrder,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

    void CancelOrder(const BfCancelOrderReq& req, BfVoid& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->CancelOrder(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->CancelOrder,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

    void QueryAccount(const BfVoid& req, BfVoid& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->QueryAccount(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->QueryAccount,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

    void QueryPosition(const BfVoid& req, BfVoid& resp)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", req_.clientid());

        grpc::Status status = stub_->QueryPosition(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("(%s)->QueryPosition,code:%d,msg:%s", qPrintable(gatewayId_), status.error_code(), status.error_message().c_str());
        }
    }

private:
    void dispatchPush(google::protobuf::Any& any)
    {
        if (any.Is<BfTickData>()) {
            BfTickData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotTick(gatewayId_, data);
        } else if (any.Is<BfPingData>()) {
            BfPingData data;
            any.UnpackTo(&data);
            BfInfo("(%s)->Connect,gotPing:%s", qPrintable(this->gatewayId_), data.message().c_str());
            emit g_sm->gatewayMgr()->gotPing(gatewayId_, data);
        } else if (any.Is<BfOrderData>()) {
            BfOrderData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotOrder(gatewayId_, data);
        } else if (any.Is<BfAccountData>()) {
            BfAccountData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotAccount(gatewayId_, data);
        } else if (any.Is<BfPositionData>()) {
            BfPositionData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotPosition(gatewayId_, data);
        } else if (any.Is<BfTradeData>()) {
            BfOrderData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotOrder(gatewayId_, data);
        } else if (any.Is<BfErrorData>()) {
            BfErrorData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotError(gatewayId_, data);
        } else if (any.Is<BfLogData>()) {
            BfLogData data;
            any.UnpackTo(&data);
            emit g_sm->gatewayMgr()->gotLog(gatewayId_, data);
        } else if (any.Is<BfNotificationData>()) {
            BfNotificationData data;
            any.UnpackTo(&data);
            switch (data.code()) {
            case NOTIFICATION_GOTCONTRACTS: {
                emit g_sm->gatewayMgr()->gotContracts(gatewayId_);
                break;
            }
            case NOTIFICATION_TRADEWILLBEGIN: {
                emit g_sm->gatewayMgr()->tradeWillBegin(gatewayId_);
                break;
            }
            default: {
                qFatal("invalid notification type");
                break;
            }
            }
        } else {
            qFatal("invalid push type");
        }
    }

private:
    std::unique_ptr<BfGatewayService::Stub> stub_;
    ::grpc::ChannelInterface* channel_;
    int pingfail_count_ = 0;
    const int deadline_ = 500;
    QString gatewayId_;
    BfConnectReq req_;

    QThread* reader_thread_ = nullptr;
};

//
// GatewayMgr
//
GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // qRegisterMetaType
    qRegisterMetaType<BfAccountData>("BfAccountData");
    qRegisterMetaType<BfPositionData>("BfPositionData");
    qRegisterMetaType<BfOrderData>("BfOrderData");
    qRegisterMetaType<BfTradeData>("BfTradeData");
    qRegisterMetaType<BfNotificationData>("BfNotificationData");
    qRegisterMetaType<BfContractData>("BfContractData");
    qRegisterMetaType<BfErrorData>("BfErrorData");
    qRegisterMetaType<BfLogData>("BfLogData");

    qRegisterMetaType<BfConnectReq>("BfConnectReq");
    qRegisterMetaType<BfGetContractReq>("BfGetContractReq");
    qRegisterMetaType<BfSendOrderReq>("BfSendOrderReq");
    qRegisterMetaType<BfCancelOrderReq>("BfCancelOrderReq");

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &GatewayMgr::onPing);
    this->pingTimer_->start();
}

void GatewayMgr::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // close timer
    this->pingTimer_->stop();
    delete this->pingTimer_;
    this->pingTimer_ = nullptr;

    // stop ....
    if (true) {
        QMutexLocker lock(&clients_mutex_);
        for (auto client : clients_) {
            delete client;
        }
        clients_.clear();
    }
}

void GatewayMgr::connectGateway(QString gatewayId, QString endpoint, const BfConnectReq& req)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
    QMutexLocker lock(&clients_mutex_);

    // gatewayclient
    GatewayClient* client = new GatewayClient(grpc::CreateChannel(endpoint.toStdString(), grpc::InsecureChannelCredentials()),
        gatewayId, req);

    // cache
    if (clients_.contains(gatewayId)) {
        auto it = clients_[gatewayId];
        delete it;
        clients_.remove(gatewayId);
    }
    clients_[gatewayId] = client;
}

void GatewayMgr::disconnectGateway(QString gatewayId)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
    QMutexLocker lock(&clients_mutex_);

    if (clients_.contains(gatewayId)) {
        BfDebug("delete gatewayclient:%s", qPrintable(gatewayId));
        GatewayClient* client = clients_[gatewayId];

        delete client;
        clients_.remove(gatewayId);
    }
}

void GatewayMgr::onGatewayDisconnected(QString gatewayId)
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
    QMutexLocker lock(&clients_mutex_);

    if (clients_.contains(gatewayId)) {
        BfDebug("free readerthread:%s", qPrintable(gatewayId));
        GatewayClient* client = clients_[gatewayId];
        client->freeReaderThread();
    }
}

void GatewayMgr::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);
    QMutexLocker lock(&clients_mutex_);

    BfPingData req, resp;
    req.set_message("cta");
    for (auto client : clients_) {
        client->Ping(req, resp);

        // NOTE(hege):做channel状态监测，如果ready而没有connect就开始connect
        if (!client->connected() && client->ready()) {
            client->Connect();
        }
    }
}

void GatewayMgr::getContract(QString gatewayId, const BfGetContractReq& req, BfContractData& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    QMutexLocker lock(&clients_mutex_);

    auto client = clients_.value(gatewayId, nullptr);
    if (client != nullptr) {
        client->GetContract(req, resp);
    }
}

void GatewayMgr::sendOrder(QString gatewayId, const BfSendOrderReq& req, BfSendOrderResp& resp)
{
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    QMutexLocker lock(&clients_mutex_);

    auto client = clients_.value(gatewayId, nullptr);
    if (client != nullptr) {
        client->SendOrder(req, resp);
    }
}

void GatewayMgr::cancelOrder(QString gatewayId, const BfCancelOrderReq& req)
{
    if (!g_sm->isCurrentOn(ServiceMgr::LOGIC)) {
        g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    }
    QMutexLocker lock(&clients_mutex_);

    BfVoid resp;
    auto client = clients_.value(gatewayId, nullptr);
    if (client != nullptr) {
        client->CancelOrder(req, resp);
    }
}

void GatewayMgr::queryAccount(QString gatewayId)
{
    if (!g_sm->isCurrentOn(ServiceMgr::LOGIC)) {
        g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    }
    QMutexLocker lock(&clients_mutex_);

    BfVoid req, resp;
    auto client = clients_.value(gatewayId, nullptr);
    if (client != nullptr) {
        client->QueryAccount(req, resp);
    }
}

void GatewayMgr::queryPosition(QString gatewayId)
{
    if (!g_sm->isCurrentOn(ServiceMgr::LOGIC)) {
        g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    }
    QMutexLocker lock(&clients_mutex_);

    BfVoid req, resp;
    auto client = clients_.value(gatewayId, nullptr);
    if (client != nullptr) {
        client->QueryPosition(req, resp);
    }
}
