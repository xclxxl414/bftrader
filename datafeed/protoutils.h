#ifndef PROTOUTILS_H
#define PROTOUTILS_H

#include "bftrader.pb.h"
#include <QString>

using namespace bftrader;

namespace ProtoUtils {

QString formatDirection(BfDirection direction);
QString formatOffset(BfOffset offset);
QString formatStatus(BfStatus status);
QString formatProduct(BfProduct product);
QString formatPeriod(BfBarPeriod period);
}

#endif // PROTOUTILS_H
