#ifndef PROTOUTILS_H
#define PROTOUTILS_H

#include "bfdatafeed.pb.h"
#include "bfgateway.pb.h"
#include <QString>

using namespace bfdatafeed;
using namespace bfgateway;

namespace ProtoUtils {

QString formatDirection(BfDirection direction);
QString formatOffset(BfOffset offset);
QString formatStatus(BfStatus status);
QString formatProduct(BfProduct product);
QString formatPeriod(BfBarPeriod period);
BfBarPeriod translatePeriod(QString period);
}

#endif // PROTOUTILS_H
