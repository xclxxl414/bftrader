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

private:
    QTimer* pingTimer_ = nullptr;

    DatafeedClient* client_ = nullptr;
};

#endif // PUSHSERVICE_H
