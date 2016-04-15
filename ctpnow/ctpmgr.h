#ifndef CTPMGR_H
#define CTPMGR_H

#include <QObject>

#include "ThostFtdcTraderApi.h"
#include "ThostFtdcMdApi.h"

class TdApi;
class MdApi;

// 1. 完成登录/自动登录/确认账单/查询可交易合约列表=
// 2. 实现自动订阅(因为策略连gateway时候，gateway并不一定连接了柜台)=
// 3. 非常策略主动取消订阅，合约的订阅会一直自动订阅下去=
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
