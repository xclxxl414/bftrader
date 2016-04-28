#include "proto_utils.h"
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

}
