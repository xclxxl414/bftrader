#pragma once

#include "gatewaymgr.h"
#include <QObject>

namespace leveldb {
class DB;
}

//
// DB
//
// map:
// 1. 委托分流，记录每一个委托属于哪个机器人：orderid:OrderExData
// 2. 成交分流，成交里面有orderid，上面一个就行了=
//
// meta: 通过getdb直接操作数据库就行了=
// 1. model本身信息，如modelid:ModelData
// 2. gateway本身信息，如gatewayid:GatewayData
// 3. robot本身信息，如robotid:RobotData
// 4. order本身信息，如orderid:OrderData
// 5. trade本身信息，如tradeid:TradeData
//
class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();
    leveldb::DB* getDb();

    //多线程安全的辅助函数=
    QString getRobotId(const BfOrderData& bfItem);
    QString getRobotId(const BfTradeData& bfItem);
    QString getGatewayId(const BfConnectReq& bfItem);
    QString getGatewayId(const QString& robotId);

signals:

private:
    void dbOpen();
    void dbClose();
    void dbInit();

private:
    leveldb::DB* db_ = nullptr;
};
