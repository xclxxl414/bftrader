#include "pushservice.h"
#include "bfproxy.grpc.pb.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfproxy;

//
// ProxyClient
//
class ProxyClient {
public:
    ProxyClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(BfProxyService::NewStub(channel))
    {
        BfDebug(__FUNCTION__);
    }
    ~ProxyClient() {}

    // ref: grpc\test\cpp\interop\interop_client.cc
    void OnPing(const BfPingData& ping)
    {
        BfPingData reply;
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
        ctx.set_deadline(deadline);
        BfDebug("ping:%s", ping.message().c_str());
        grpc::Status status = stub_->OnPing(&ctx, ping, &reply);
        if (!status.ok()) {
            BfDebug("stub_->OnPing fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return;
        }
        BfDebug("pong:%s", reply.message().c_str());
    }

private:
    std::unique_ptr<BfProxyService::Stub> stub_;
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
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

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

    ProxyClient* proxyClient = new ProxyClient(grpc::CreateChannel(
        endpoint.toStdString(), grpc::InsecureChannelCredentials()));

    proxyClients_[proxyId] = proxyClient;

    //
    // TODO(hege): remove debug
    //
    google::protobuf::Arena arena;
    BfPingData* data = google::protobuf::Arena::CreateMessage<BfPingData>(&arena);
    data->set_message("pingpong");
    proxyClient->OnPing(*data);
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
