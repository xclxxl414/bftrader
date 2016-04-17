#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QObject>
#include <QMap>

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
    void onRobotConnected(QString robotId,qint32 endpoint);

private:
    QMap<QString,RobotClient*> robotClients_;
};

#endif // PUSHSERVICE_H
