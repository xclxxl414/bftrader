#ifndef DATAPUMP_H
#define DATAPUMP_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QThread>
#include <QList>

class RingBuffer;
class DbService;
namespace leveldb {
class DB;
}

//CTP
class DataPump : public QObject {
    Q_OBJECT
public:
    explicit DataPump(QObject* parent = 0);
    void init();
    void shutdown();
    void putTick(void* tick);
    void putInstrument(void* instrument);
    RingBuffer* getRingBuffer(QString id);
    void initRingBuffer(QStringList ids);
    void freeRingBuffer();

signals:
    void gotTick(void* tick, int indexRb, void* rb);

public slots:
    void initInstrumentLocator();

private:
    void* putTickToRingBuffer(void* tick, int& indexRb, RingBuffer*& rb);
    void fixTickMs(void* tick, int indexRb, RingBuffer* rb);
    void initTickLocator(QString id);
    void loadRingBufferFromBackend(QStringList ids);
    bool shouldSkipTick(void* tick);

private:
    QMap<QString, RingBuffer*> rbs_;
    const int ringBufferLen_ = 256;
};

#endif // DATAPUMP_H
