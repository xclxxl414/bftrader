#pragma once

#include "gatewaymgr.h"
#include <QObject>

namespace leveldb {
class DB;
}

//
// DB
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
