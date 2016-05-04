#pragma once

#include "gatewaymgr.h"
#include <QObject>

namespace leveldb {
class DB;
}

// DB
class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();

    //多线程安全=
    QString getRobotId(const BfOrderData& bfItem);
    QString getRobotId(const BfTradeData& bfItem);
    QString getGatewayId(const BfConnectReq& bfItem);
    QString getGatewayId(const QString& robotId);

signals:

public slots:
    void dbOpen();
    void dbClose();

private:
    leveldb::DB* db_ = nullptr;
};
