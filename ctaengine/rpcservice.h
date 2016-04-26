#ifndef RPCSERVICE_H
#define RPCSERVICE_H

#include <QObject>

//IO
class RpcService : public QObject {
    Q_OBJECT
public:
    explicit RpcService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:

private:
};

#endif // RPCSERVICE_H
