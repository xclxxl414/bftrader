#ifndef CTAMGR_H
#define CTAMGR_H

#include <QObject>

// FLOGIC
class CtaMgr : public QObject {
    Q_OBJECT
public:
    explicit CtaMgr(QObject* parent = 0);
    void init();
    void shutdown();

signals:
};

#endif // CTAMGR_H
