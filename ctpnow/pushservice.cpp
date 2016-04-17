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

    void OnTick(const BfTickData& tick)
    {
        BfVoid reply;
        grpc::ClientContext ctx;
        grpc::Status status = stub_->OnTick(&ctx, tick, &reply);
        if (status.ok()) {
            g_sm->logger()->info("stub_->OnTick ok");
        } else {
            g_sm->logger()->info("stub_->OnTick fail");
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
    for(int i = 0;i<robotClients_.size();i++){
        auto robotClient = robotClients_.values().at(i);
        delete robotClient;
    }
    robotClients_.clear();
}

void PushService::onRobotConnected(QString robotId, qint32 endpoint)
{
    g_sm->logger()->info(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);
    QString target = QString().sprintf("localhost:%d",endpoint);

    RobotClient* robotClient = new RobotClient(grpc::CreateChannel(
        target.toStdString(), grpc::InsecureChannelCredentials()));

    robotClients_[robotId] = robotClient;

    //
    // TODO(hege): remove debug
    //
    BfTickData data;
    robotClient->OnTick(data);
}
