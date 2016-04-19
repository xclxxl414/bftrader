#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QMap>
#include <QObject>

class RobotClient;

//IO
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:
    void onRobotConnected(QString robotId, QString robotIp, qint32 robotPort);

private:
    QMap<QString, RobotClient*> robotClients_;
};

#endif // PUSHSERVICE_H
