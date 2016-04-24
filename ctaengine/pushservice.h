#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include <QObject>

//IO
class PushService : public QObject {
    Q_OBJECT
public:
    explicit PushService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:

private:
};

#endif // PUSHSERVICE_H
