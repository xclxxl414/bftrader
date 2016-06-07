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
    case PERIOD_S01:
        return "s01";
    case PERIOD_S03:
        return "s03";
    case PERIOD_S05:
        return "s05";
    case PERIOD_S10:
        return "s10";
    case PERIOD_S15:
        return "s15";
    case PERIOD_S30:
        return "s30";
    case PERIOD_M01:
        return "m01";
    case PERIOD_M03:
        return "m03";
    case PERIOD_M05:
        return "m05";
    case PERIOD_M10:
        return "m10";
    case PERIOD_M15:
        return "m15";
    case PERIOD_M30:
        return "m30";
    case PERIOD_H01:
        return "h01";
    case PERIOD_D01:
        return "d01";
    case PERIOD_W01:
        return "w01";
    case PERIOD_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid period");
    }

    return "unknown";
}

BfBarPeriod translatePeriod(QString period)
{
    if (period == "s01")
        return PERIOD_S01;
    else if (period == "s03")
        return PERIOD_S03;
    else if (period == "s05")
        return PERIOD_S05;
    else if (period == "s10")
        return PERIOD_S10;
    else if (period == "s15")
        return PERIOD_S15;
    else if (period == "s30")
        return PERIOD_S30;
    else if (period == "m01")
        return PERIOD_M01;
    else if (period == "m03")
        return PERIOD_M03;
    else if (period == "m05")
        return PERIOD_M05;
    else if (period == "m10")
        return PERIOD_M10;
    else if (period == "m15")
        return PERIOD_M15;
    else if (period == "m30")
        return PERIOD_M30;
    else if (period == "h01")
        return PERIOD_H01;
    else if (period == "d01")
        return PERIOD_D01;
    else if (period == "w01")
        return PERIOD_W01;
    else
        qFatal("invalid period");
    return PERIOD_UNKNOWN;
}
}
