#include "pushservice.h"
#include "bfrobot.grpc.pb.h"
#include "logger.h"
#include "servicemgr.h"
#include <grpc++/grpc++.h>

using namespace bftrader;
using namespace bftrader::bfrobot;

//
// RobotClient
//
class RobotClient {
public:
    RobotClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(BfRobotService::NewStub(channel))
    {
        g_sm->logger()->info(__FUNCTION__);
    }
    ~RobotClient() {}

    // ref: grpc\test\cpp\interop\interop_client.cc
    void OnTick(const BfTickData& tick)
    {
        BfVoid reply;
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
        ctx.set_deadline(deadline);
        grpc::Status status = stub_->OnTick(&ctx, tick, &reply);
        if (!status.ok()) {
            g_sm->logger()->info(QString().sprintf("stub_->OnTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str()));
        }
    }

private:
    std::unique_ptr<BfRobotService::Stub> stub_;
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
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
}

void PushService::shutdown()
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // delete all robotclient
    for (int i = 0; i < robotClients_.size(); i++) {
        auto robotClient = robotClients_.values().at(i);
        delete robotClient;
    }
    robotClients_.clear();
}

void PushService::onRobotConnected(QString robotId, QString robotIp, qint32 robotPort)
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString endpoint = QString().sprintf("%s:%d", robotIp.toStdString().c_str(), robotPort);

    RobotClient* robotClient = new RobotClient(grpc::CreateChannel(
        endpoint.toStdString(), grpc::InsecureChannelCredentials()));

    robotClients_[robotId] = robotClient;

    //
    // TODO(hege): remove debug
    //
    google::protobuf::Arena arena;
    BfTickData* data = google::protobuf::Arena::CreateMessage<BfTickData>(&arena);
    robotClient->OnTick(*data);
}
