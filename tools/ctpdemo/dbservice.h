#ifndef DBSERVICE_H
#define DBSERVICE_H

#include <QObject>

namespace leveldb {
class DB;
}

//
// 分三个数据库,instrument tick bar
// instrument:合约信息，key为instrument
// tick:分笔数据，每1分钟的数据打包存储，key为instrument-yyyy-mm-dd-hh-mm-ss
// bar:bar数据，只存储1分钟的bar，key为instrument-yyyy-mm-dd-hh-mm-ss
//
// todo(hege):修改此类为dbservice，订阅ctpmgr的数据，完成三类数据的写盘，并提供读取服务（rpcservice需要）=

// dbthread (可以在多线程上读取和写入=)=
class DbService : public QObject{
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    ~DbService();
    void init();
    void shutdown();
    leveldb::DB* getInstrumentDB();
    leveldb::DB* getTickDB();
    leveldb::DB* getBarDB();
    void initInstrumentLocator();
    void initTickLocator(QString id);
    void initBarLocator(QString id);
signals:

public slots:
    void putTick(void* tick, int indexRb, void* rb);
    void putInstrument(void* instrument);

private:
    void openInstrumentDB();
    void closeInstrumentDB();
    void openBarDB();
    void closeBarDB();
    void openTickDB();
    void closeTickDB();

private:
    leveldb::DB* instrument_db_ = nullptr;
    leveldb::DB* tick_db_ = nullptr;
    leveldb::DB* bar_db_ = nullptr;
};


#endif // DBSERVICE_H
