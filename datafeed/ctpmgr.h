#ifndef CTPMGR_H
#define CTPMGR_H

#include <QObject>

#include "bftrader.pb.h"
using namespace bftrader;

class CtpMgr : public QObject {
    Q_OBJECT
public:
    explicit CtpMgr(QObject* parent = 0);
    void init();
    void shutdown();
};

#endif // CTPMGR_H
