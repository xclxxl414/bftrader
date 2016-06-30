#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QMutex>
#include <QObject>

class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(QObject* parent = 0);
    void init();
    void shutdown();
    Q_INVOKABLE void log(QString msg);
    static void startExitMonitor();
    static void stopExitMonitor();

signals:
    void gotLog(QString when, QString msg);

private:
    QFile log_;
    QMutex mutex_;
};

#endif // LOGGER_H
