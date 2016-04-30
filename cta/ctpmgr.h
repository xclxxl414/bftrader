#ifndef CTPMGR_H
#define CTPMGR_H

#include <QObject>

class CtpMgr : public QObject {
    Q_OBJECT
public:
    explicit CtpMgr(QObject* parent = 0);
    void init();
    void shutdown();

signals:

};

#endif // CTPMGR_H
