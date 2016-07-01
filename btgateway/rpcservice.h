#ifndef RPCSERVICE_H
#define RPCSERVICE_H

#include <QObject>

namespace grpc {
class Server;
}

// RPC
class RpcService : public QObject {
    Q_OBJECT
public:
    explicit RpcService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void start();
    void stop();

private slots:
    void onGatewayThreadStarted();

private:
    QThread* gatewayThread_ = nullptr;
    grpc::Server* grpcServer_ = nullptr;
};

#endif // RPCSERVICE_H
