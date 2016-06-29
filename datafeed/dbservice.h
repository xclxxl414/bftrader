#pragma once

#include "gatewaymgr.h"
#include <QObject>
#include <grpc++/grpc++.h>

namespace leveldb {
class DB;
}
class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();
    leveldb::DB* getDb();

signals:
    void opened();

public slots:
    // RPC
    void insertTick(const BfTickData& bfItem);
    void insertBar(const BfBarData& bfItem);
    void insertContract(const BfContractData& bfItem);

    void getTick(const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer);
    void getBar(const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer);
    void getContract(const BfGetContractReq* request, ::grpc::ServerWriter<BfContractData>* writer);

    void deleteTick(const BfDeleteTickReq& bfReq);
    void deleteBar(const BfDeleteBarReq& bfReq);
    void deleteContract(const BfDeleteContractReq& bfReq);

    void cleanAll();

    // UI
    void dbCompact();

private:
    void dbOpen();
    void dbClose();
    void dbInit();
    void dbReset();

    void getTickFromCount(const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer);
    void getTickFromTo(const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer);
    void getTickCountTo(const BfGetTickReq* request, ::grpc::ServerWriter<BfTickData>* writer);

    void getBarFromCount(const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer);
    void getBarFromTo(const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer);
    void getBarCountTo(const BfGetBarReq* request, ::grpc::ServerWriter<BfBarData>* writer);

private:
    leveldb::DB* db_ = nullptr;
};
