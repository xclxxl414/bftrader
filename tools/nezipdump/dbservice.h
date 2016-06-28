#pragma once

#include <QObject>

class DbService : public QObject {
    Q_OBJECT
public:
    explicit DbService(QObject* parent = 0);
    void init();
    void shutdown();

signals:

public slots:

private:
};
