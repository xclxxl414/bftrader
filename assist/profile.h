#ifndef PROFILE_H
#define PROFILE_H

#include <QMap>
#include <QObject>
#include <QVariant>

class QLevelDB;
class QLevelDBBatch;

class Profile : public QObject {
    Q_OBJECT
public:
    explicit Profile(QObject* parent = 0);
    void init();
    void shutdown();

public:
    QVariant get(QString k, QVariant defaultValue = QVariant());
    void put(QString k, QVariant v);
    void commit();

    static QString dbPath();
    static QString logPath();
    static QString appName();

signals:
    void keyValueChanged(QString key, QVariant value);

public slots:

private:
    QVariantMap store_;
    QString path_;
    bool dirty_ = false;
};

#endif // PROFILE_H
