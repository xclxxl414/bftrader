#pragma once

#include "gatewaymgr.h"
#include <QObject>

namespace leveldb {
class DB;
}
class DatafeedClient;

//
// TODO(hege):
// 1.ringbuffer实现最近tick
// 2.map实现contracts
// 3.接入datafeed，实现dataframe

//
// DB：order trade datafeed dataframe ringbuffer contracts
//
class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();

    //线程安全=
    leveldb::DB* getDb();
    bool getTick(const BfGetTickReq& req, QList<BfTickData>& resp);
    bool getBar(const BfGetBarReq& req, QList<BfBarData>& resp);
    bool getContract(const BfGetContractReq& req, QList<BfContractData>& resp);

public slots:
    void connectDatafeed(QString endpoint, QString clientId);
    void disconnectDatafeed();
    void onPing();

signals:

private:
    void dbOpen();
    void dbClose();
    void dbInit();

private:
    leveldb::DB* db_ = nullptr;

    QTimer* pingTimer_ = nullptr;
    DatafeedClient* client_ = nullptr;
};
