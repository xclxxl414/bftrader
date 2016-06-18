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
private:
    void dbOpen();
    void dbClose();
    void dbInit();
    void dbReset();

private:
    leveldb::DB* db_ = nullptr;
};
