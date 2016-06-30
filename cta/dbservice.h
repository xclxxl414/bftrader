#pragma once

#include "gatewaymgr.h"
#include <QObject>

namespace leveldb {
class DB;
}

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
    leveldb::DB* getDb();

signals:

private:
    void dbOpen();
    void dbClose();
    void dbInit();

private:
    leveldb::DB* db_ = nullptr;
};
