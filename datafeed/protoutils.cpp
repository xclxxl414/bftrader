#include "protoutils.h"
#include "servicemgr.h"

///////////
namespace ProtoUtils {

QString formatDirection(BfDirection direction)
{
    switch (direction) {
    case DIRECTION_LONG:
        return "long";
    case DIRECTION_SHORT:
        return "short";
    case DIRECTION_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid directioni");
    }

    return "unknown";
}

QString formatOffset(BfOffset offset)
{
    switch (offset) {
    case OFFSET_CLOSE:
        return "close";
    case OFFSET_CLOSETODAY:
        return "closetoday";
    case OFFSET_CLOSEYESTERDAY:
        return "closeyesterday";
    case OFFSET_OPEN:
        return "open";
    case OFFSET_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid offset");
    }

    return "unknown";
}

QString formatStatus(BfStatus status)
{
    switch (status) {
    case STATUS_ALLTRADED:
        return "alltraded";
    case STATUS_CANCELLED:
        return "cancelled";
    case STATUS_NOTTRADED:
        return "nottraced";
    case STATUS_PARTTRADED:
        return "parttraded";
    case STATUS_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid status");
    }

    return "unknown";
}

QString formatProduct(BfProduct product)
{
    switch (product) {
    case PRODUCT_EQUITY:
        return "equity";
    case PRODUCT_FUTURES:
        return "future";
    case PRODUCT_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid product");
    }

    return "unknown";
}

QString formatPeriod(BfBarPeriod period)
{
    switch (period) {
    case PERIOD_S1:
        return "s1";
    case PERIOD_S3:
        return "s3";
    case PERIOD_S5:
        return "s5";
    case PERIOD_S10:
        return "s10";
    case PERIOD_S15:
        return "s15";
    case PERIOD_S30:
        return "s30";
    case PERIOD_M1:
        return "m1";
    case PERIOD_M3:
        return "m3";
    case PERIOD_M5:
        return "m5";
    case PERIOD_M10:
        return "m10";
    case PERIOD_M15:
        return "m15";
    case PERIOD_M30:
        return "m30";
    case PERIOD_H1:
        return "h1";
    case PERIOD_D1:
        return "d1";
    case PERIOD_W1:
        return "w1";
    case PERIOD_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid period");
    }

    return "unknown";
}
}
