#include "pushservice.h"
#include "servicemgr.h"
#include "DataWriteQueue.h"
#include "grpc++/grpc++.h"
#include "bfgateway.pb.h"
#include "bfdatafeed.grpc.pb.h"
#include "gatewaymgr.h"
#include <QDateTime>

using namespace bfdatafeed;
using namespace bfgateway;

//======
//
// DatafeedClient (sync)
//
class DatafeedClient {
public:
    DatafeedClient(std::shared_ptr<grpc::Channel> channel, QString clientId)
        : stub_(BfDatafeedService::NewStub(channel))
        , clientId_(clientId)
        , channel_(channel.get())
    {
        BfDebug(__FUNCTION__);
    }
    ~DatafeedClient()
    {
        BfDebug(__FUNCTION__);
    }

    bool ready()
    {
        if (GRPC_CHANNEL_READY == channel_->GetState(true)) {
            return true;
        }
        return false;
    }
public:
    // ref: grpc\test\cpp\interop\interop_client.cc
    bool Ping(const BfPingData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfPingData resp;
        grpc::Status status = stub_->Ping(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("Datafeed->Ping fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        if (req.message() != resp.message()) {
            BfError("Datafeed->Ping fail,ping:%s,pong:%s", req.message().c_str(), resp.message().c_str());
            return false;
        }

        return true;
    }

    bool InsertTick(const BfTickData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid resp;
        grpc::Status status = stub_->InsertTick(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("Datafeed->InsertTick fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

    bool InsertBar(const BfBarData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid resp;
        grpc::Status status = stub_->InsertBar(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("Datafeed->InsertBar fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

    bool InsertContract(const BfContractData& req)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid resp;
        grpc::Status status = stub_->InsertContract(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("Datafeed->InsertContract fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

    bool GetContract(const BfGetContractReq& req,QList<BfContractData>& resps)
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(5 * deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        std::unique_ptr< ::grpc::ClientReader< BfContractData >> reader = stub_->GetContract(&ctx, req);
        for(;;){
            BfContractData resp;
            bool ok = reader->Read(&resp);
            if (ok) {
                resps.append(resp);
            }else{
                grpc::Status status = reader->Finish();
                if (!status.ok()) {
                    BfError("Datafeed->GetContract fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
                    return false;
                }
                break;
            }
        }

        return true;
    }

    bool CleanAll()
    {
        grpc::ClientContext ctx;
        std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(deadline_);
        ctx.set_deadline(deadline);
        ctx.AddMetadata("clientid", clientId_.toStdString());

        BfVoid req,resp;
        grpc::Status status = stub_->CleanAll(&ctx, req, &resp);
        if (!status.ok()) {
            BfError("Datafeed->CleanAll fail,code:%d,msg:%s", status.error_code(), status.error_message().c_str());
            return false;
        }

        return true;
    }

private:
    std::unique_ptr<BfDatafeedService::Stub> stub_;
    ::grpc::ChannelInterface* channel_;
    QString clientId_;
    const int deadline_ = 1000;
};

//======
PushService::PushService(QObject* parent)
    : QObject(parent)
{
}

void PushService::init()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // datafeed client
    client_ = new DatafeedClient(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()),"nezipdump");

    // start timer
    this->pingTimer_ = new QTimer(this);
    this->pingTimer_->setInterval(5 * 1000);
    QObject::connect(this->pingTimer_, &QTimer::timeout, this, &PushService::onPing);
    this->pingTimer_->start();
}

void PushService::shutdown()
{
    BfDebug(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // close timer
    this->pingTimer_->stop();
    delete this->pingTimer_;
    this->pingTimer_ = nullptr;

    // free client
    delete client_;
    client_ = nullptr;
}

void PushService::onPing()
{
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    // check
    if(!recvFinished_){
        recvFinished_ = checkRecvFinished();
    }

    // push
    if(recvFinished_){
        // ping datafeed
        BfPingData req;
        req.set_message("nezipdump");
        bool ok = client_->Ping(req);

        // ready是没必要的，用ping检测最好=
        if(ok  || client_->ready()){
            pushToDatafeed();
        }
    }
}

bool PushService::checkRecvFinished()
{
    int daysize = 0;
    int min1size = 0;
    int min5size = 0;
    int ticksize = 0;
    gDataWriteQueue.getdatasize(daysize,min1size,min5size,ticksize);

    if(daysize != daysize_ || min1size!=min1size_ || min5size!=min5size_ || ticksize!=ticksize_){
        BfInfo("recv: tick(%d),m01(%d),m05(%d),day(%d)",ticksize,min1size,min5size,daysize);
        daysize_ = daysize;
        min1size_ = min1size;
        min5size_ = min5size;
        ticksize_ = ticksize;
     }else{
        //5秒内无新数据接收到，认为接收完毕
        if(daysize_!=0 || min1size_!=0 || min5size_!=0 || ticksize_!=0){
            BfInfo("recv finished!");
            return true;
        }
    }

    return false;
}

void PushService::pushToDatafeed()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    if(!pushIndexFinished_){
        pushIndexFinished_ =  pushIndexData();
        if(!pushIndexFinished_){
            return;
        }
    }

    for(;;){
        int daysize = 0;
        int min1size = 0;
        int min5size = 0;
        int ticksize = 0;
        gDataWriteQueue.getdatasize(daysize,min1size,min5size,ticksize);

        if(daysize == 0 && min1size==0 && min5size==0 && ticksize==0){
            BfInfo("push finished,please exit app!");
            this->pingTimer_->stop();
            break;
        }
        BfInfo("rest: tick(%d),m01(%d),m05(%d),day(%d)",ticksize,min1size,min5size,daysize);

        if(daysize){
            pushDayData();
        }
        if(min1size){
            pushMin1Data();
        }
        if(min5size){
            pushMin5Data();
        }
        if(ticksize){
            pushTickData();
        }
    }
}

bool PushService::pushIndexData()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    QMap<QString,AskDataTag> tags = g_sm->gatewayMgr()->tags();
    for(auto tag: tags){
        if(tag.index){
            QList<BfContractData> resps;
            BfGetContractReq req;
            req.set_symbol(tag.ctpCloner.toStdString());
            req.set_exchange(tag.ctpExchange.toStdString());
            bool ok = client_->GetContract(req,resps);
            if(!ok){
                BfError("GetContract error");
                return false;
            }
            if(resps.length()==0){
                BfError("GetContract return item:0");
                return false;
            }
            BfContractData resp = resps.at(0);
            resp.set_symbol(tag.ctpSymbol.toStdString());
            resp.set_name("index");
            ok = client_->InsertContract(resp);
            if(!ok){
                BfError("InsertContract error");
                return false;
            }
        }
    }

    return true;
}

void PushService::pushDayData()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    std::list<KLineData> dayData;
    gDataWriteQueue.get_daydata(dayData);

    QMap<QString,AskDataTag> tags = g_sm->gatewayMgr()->tags();
    for(auto data:dayData){
        BfBarData req;
        std::string label = data.mkt + data.code;
        if(tags.contains(label.c_str())){
            AskDataTag tag = tags.value(label.c_str());

            //代码相关=
            req.set_exchange(tag.ctpExchange.toStdString());
            req.set_symbol(tag.ctpSymbol.toStdString());

            //周期=
            req.set_period(PERIOD_D01);

            //成交数据=
            req.set_actiondate(QDateTime::fromTime_t(data.time).toString("yyyyMMdd").toStdString());
            req.set_bartime(QDateTime::fromTime_t(data.time).toString("hh:mm:ss").toStdString());
            req.set_volume(data.volume);
            req.set_openinterest(data.amount);
            req.set_lastvolume(1);

            //OHLC
            req.set_openprice(data.open);
            req.set_highprice(data.high);
            req.set_lowprice(data.low);
            req.set_closeprice(data.close);

            bool ok = client_->InsertBar(req);
            if(!ok){
                BfError("InsertBar error");
                return;
            }
        }
    }
}

void PushService::pushMin1Data()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    std::list<KLineData> m1Data;
    gDataWriteQueue.get_min1data(m1Data);

    QMap<QString,AskDataTag> tags = g_sm->gatewayMgr()->tags();
    for(auto data:m1Data){
        BfBarData req;
        std::string label = data.mkt + data.code;
        if(tags.contains(label.c_str())){
            AskDataTag tag = tags.value(label.c_str());

            //代码相关=
            req.set_exchange(tag.ctpExchange.toStdString());
            req.set_symbol(tag.ctpSymbol.toStdString());

            //周期=
            req.set_period(PERIOD_M01);

            //成交数据=
            req.set_actiondate(QDateTime::fromTime_t(data.time).toString("yyyyMMdd").toStdString());
            req.set_bartime(QDateTime::fromTime_t(data.time).toString("hh:mm:ss").toStdString());
            req.set_volume(data.volume);
            req.set_openinterest(data.amount);
            req.set_lastvolume(1);

            //OHLC
            req.set_openprice(data.open);
            req.set_highprice(data.high);
            req.set_lowprice(data.low);
            req.set_closeprice(data.close);

            bool ok = client_->InsertBar(req);
            if(!ok){
                BfError("InsertBar error");
                return;
            }
        }
    }
}

void PushService::pushMin5Data()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    std::list<KLineData> m5Data;
    gDataWriteQueue.get_min5data(m5Data);

    QMap<QString,AskDataTag> tags = g_sm->gatewayMgr()->tags();
    for(auto data:m5Data){
        BfBarData req;
        std::string label = data.mkt + data.code;
        if(tags.contains(label.c_str())){
            AskDataTag tag = tags.value(label.c_str());

            //代码相关=
            req.set_exchange(tag.ctpExchange.toStdString());
            req.set_symbol(tag.ctpSymbol.toStdString());

            //周期=
            req.set_period(PERIOD_M05);

            //成交数据=
            req.set_actiondate(QDateTime::fromTime_t(data.time).toString("yyyyMMdd").toStdString());
            req.set_bartime(QDateTime::fromTime_t(data.time).toString("hh:mm:ss").toStdString());
            req.set_volume(data.volume);
            req.set_openinterest(data.amount);
            req.set_lastvolume(1);

            //OHLC
            req.set_openprice(data.open);
            req.set_highprice(data.high);
            req.set_lowprice(data.low);
            req.set_closeprice(data.close);

            bool ok = client_->InsertBar(req);
            if(!ok){
                BfError("InsertBar error");
                return;
            }
        }
    }
}

void PushService::pushTickData()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::PUSH);

    std::list<TickData> tickData;
    gDataWriteQueue.get_tickdata(tickData);

    QMap<QString,AskDataTag> tags = g_sm->gatewayMgr()->tags();
    for(auto data:tickData){
        BfTickData req;
        std::string label = data.mkt + data.code;
        if(tags.contains(label.c_str())){
            AskDataTag tag = tags.value(label.c_str());

            //代码相关=
            req.set_exchange(tag.ctpExchange.toStdString());
            req.set_symbol(tag.ctpSymbol.toStdString());

            //成交数据=
            req.set_actiondate(QDateTime::fromTime_t(data.time).toString("yyyyMMdd").toStdString());
            req.set_ticktime(QDateTime::fromTime_t(data.time).toString("hh:mm:ss.zzz").toStdString());
            req.set_lastprice(data.close);
            req.set_volume(data.volume);
            req.set_openinterest(data.amount);
            req.set_lastvolume(1);

            // 常规行情
            req.set_openprice(0);
            req.set_highprice(0);
            req.set_lowprice(0);
            req.set_precloseprice(0);
            req.set_upperlimit(0);
            req.set_lowerlimit(0);

            // x档行情
            req.set_bidprice1(data.pricesell[0]);
            req.set_askprice1(data.pricebuy[0]);
            req.set_bidvolume1(data.volsell[0]);
            req.set_askvolume1(data.volbuy[0]);

            bool ok = client_->InsertTick(req);
            if(!ok){
                BfError("InsertTick error");
                return;
            }
        }
    }
}
