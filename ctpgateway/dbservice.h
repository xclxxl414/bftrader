#pragma once

#include <QObject>

namespace leveldb {
class DB;
}
class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void dbOpen();
    void dbInit();
    void dbClose();

private:
    leveldb::DB* db_ = nullptr;
};
