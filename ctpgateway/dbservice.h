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
    leveldb::DB* getDb();

private:
    void dbOpen();
    void dbClose();
    void batchWriteTicks();

private slots:
    void onGotContracts(QStringList ids, QStringList idsAll);
    void onGotTick(void* curTick, void* preTick);
    void onTradeWillBegin();

private:
    leveldb::DB* db_ = nullptr;

    struct TickPair {
        void* curTick;
        void* preTick;
    };
    static const int tickArrayLen_ = 128;
    TickPair tickArray[tickArrayLen_] = { 0 };
    int tickCount_ = 0;
};
