#ifndef CTPMGR_H
#define CTPMGR_H

#include <QObject>

#include "ThostFtdcTraderApi.h"
#include "ThostFtdcMdApi.h"

class TdApi;
class MdApi;

// 完成登录/自动登录/确认账单/查询可交易合约列表，然后通知策略logined？
// 不实现自动订阅，只发送login/logout给策略，策略再发过来订阅什么？
// 策略掉线后，取消策略的订阅，策略上线后会发订阅？
class CtpMgr : public QObject
{
    Q_OBJECT
public:
    explicit CtpMgr(QObject *parent = 0);
    void init();
    void shutdown();

    // tdapi/mdpai有信号和巢，需要的自己去弄=
    TdApi* tdapi();
    MdApi* mdapi();

signals:

public slots:
    void showVersion();
    void start();
    void stop();

private:
    TdApi* tdapi_ = nullptr;
    MdApi* mdapi_ = nullptr;
};

#endif // CTPMGR_H
