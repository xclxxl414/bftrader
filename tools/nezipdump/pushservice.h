#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QObject>
#include <QTimer>

class DatafeedClient;
// PUSH
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void onPing();
    void pushToDatafeed();

private:
    bool checkRecvFinished();
    void pushDayData();
    void pushMin1Data();
    void pushMin5Data();
    void pushTickData();
    bool pushIndexData();

private:
    QTimer* pingTimer_ = nullptr;

    int daysize_ = 0;
    int min1size_ = 0;
    int min5size_ = 0;
    int ticksize_ = 0;
    bool recvFinished_ = false;
    bool pushIndexFinished_ = false;

    DatafeedClient* client_ = nullptr;
};

#endif // PUSHSERVICE_H
