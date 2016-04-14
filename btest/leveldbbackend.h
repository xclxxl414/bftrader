#ifndef LEVELDBBACKEND_H
#define LEVELDBBACKEND_H

#include <QObject>

namespace leveldb{
class DB;
}
class LeveldbBackend : public QObject
{
    Q_OBJECT
public:
    explicit LeveldbBackend(QObject *parent = 0);
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

#endif // LEVELDBBACKEND_H
