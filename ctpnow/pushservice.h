#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QMap>
#include <QObject>

class ProxyClient;

//IO
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void onProxyConnect(QString proxyId, QString proxyIp, qint32 proxyPort);
    void onProxyClose(QString proxyId);

private:
    QMap<QString, ProxyClient*> proxyClients_;
};

#endif // PUSHSERVICE_H
