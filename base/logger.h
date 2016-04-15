#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);
    void init();
    void shutdown();
    Q_INVOKABLE void info(QString msg);
    static void startExitMonitor();
    static void stopExitMonitor();

signals:
    void gotInfo(QString when,QString msg);

private:
    QFile log_;
    QMutex mutex_;
};

#endif // LOGGER_H
