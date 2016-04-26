#include "ctp_utils.h"
#include "ThostFtdcUserApiStruct.h"
#include "encode_utils.h"
#include "servicemgr.h"

///////////
namespace CtpUtils {
char translatePriceType(BfPriceType priceType)
{
    switch (priceType) {
    case PRICETYPE_LIMITPRICE:
        return THOST_FTDC_OPT_LimitPrice;
    case PRICETYPE_MARKETPRICE:
        return THOST_FTDC_OPT_AnyPrice;
    default:
        qFatal("invalid BfPriceType");
    }
    return '0';
}

char translateOffset(BfOffset offset)
{
    switch (offset) {
    case OFFSET_OPEN:
        return THOST_FTDC_OF_Open;
    case OFFSET_CLOSE:
        return THOST_FTDC_OF_Close;
    case OFFSET_CLOSETODAY:
        return THOST_FTDC_OF_CloseToday;
    case OFFSET_CLOSEYESTERDAY:
        return THOST_FTDC_OF_CloseYesterday;
    default:
        qFatal("invalid offset");
    }
    return '0';
}

char translateDirection(BfDirection direction)
{
    switch (direction) {
    case DIRECTION_LONG:
        return THOST_FTDC_D_Buy;
    case DIRECTION_SHORT:
        return THOST_FTDC_D_Sell;
    default:
        qFatal("invalid direction");
    }
    return '0';
}

BfPriceType translatePriceType(char priceType)
{
    switch (priceType) {
    case THOST_FTDC_OPT_LimitPrice:
        return PRICETYPE_LIMITPRICE;
    case THOST_FTDC_OPT_AnyPrice:
        return PRICETYPE_MARKETPRICE;
    default:
        qFatal("invalid BfPriceType");
    }
    return PRICETYPE_UNKONWN;
}

BfOffset translateOffset(char offset)
{
    switch (offset) {
    case THOST_FTDC_OF_Open:
        return OFFSET_OPEN;
    case THOST_FTDC_OF_Close:
        return OFFSET_CLOSE;
    case THOST_FTDC_OF_CloseToday:
        return OFFSET_CLOSETODAY;
    case THOST_FTDC_OF_CloseYesterday:
        return OFFSET_CLOSEYESTERDAY;
    default:
        BfDebug("invalid offset");
    }
    return OFFSET_UNKNOWN;
}

// 方向类型映射=
BfDirection translateDirection(char direction)
{
    switch (direction) {
    case THOST_FTDC_D_Buy:
        return DIRECTION_LONG;
    case THOST_FTDC_D_Sell:
        return DIRECTION_SHORT;
    default:
        BfDebug("invalid direction");
    }
    return DIRECTION_UNKNOWN;
}

// 持仓类型映射=
BfDirection translatePosiDirection(char direction)
{
    switch (direction) {
    case THOST_FTDC_PD_Net:
        return DIRECTION_NET;
    case THOST_FTDC_PD_Long:
        return DIRECTION_LONG;
    case THOST_FTDC_PD_Short:
        return DIRECTION_SHORT;
    default:
        BfDebug("invalid direction");
    }
    return DIRECTION_UNKNOWN;
}

BfStatus translateStatus(char status)
{
    switch (status) {
    case THOST_FTDC_OST_AllTraded:
        return STATUS_ALLTRADED;
    case THOST_FTDC_OST_PartTradedQueueing:
        return STATUS_PARTTRADED;
    case THOST_FTDC_OST_NoTradeQueueing:
        return STATUS_NOTTRADED;
    case THOST_FTDC_OST_Canceled:
        return STATUS_CANCELLED;
    default:
        BfDebug("invalid status");
    }
    return STATUS_UNKNOWN;
}

BfProduct translateProduct(char product)
{
    switch (product) {
    case THOST_FTDC_PC_Futures:
        return PRODUCT_FUTURES;
    default:
        BfDebug("invalid product");
    }

    return PRODUCT_UNKNOWN;
}

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

char* getExchangeFromContract(void* contract)
{
    auto data = (CThostFtdcInstrumentField*)contract;
    return data->ExchangeID;
}

int getVolumeMultipleFromContract(void* contract)
{
    auto data = (CThostFtdcInstrumentField*)contract;
    return data->VolumeMultiple;
}

/*
    // 代码编号相关
    string symbol = 1;          // 合约代码
    string exchange = 2;        // 交易所代码
    string name = 3;            // 合约中文名-utf8

    BfProduct productClass = 4; // 合约类型
    int32 volumeMultiple = 5;   // 合约数量乘数
    double priceTick = 6;       // 合约最小价格TICK

    int32 maxLimit = 7;         // 限价单最大下单量
    int32 minLimit = 8;         // 限价单最小下单量
    int32 maxMarket = 9;        // 市价单最大下单量
    int32 minMartet = 10;       // 市价单最小下单量
*/
void translateContract(void* from, BfContractData* to)
{
    auto data = (CThostFtdcInstrumentField*)from;

    to->set_symbol(data->InstrumentID);
    to->set_exchange(data->ExchangeID);
    //gbk会导致protobuf反序列化失败=
    to->set_name(gbk2utf16(data->InstrumentName).toStdString());

    to->set_productclass(translateProduct(data->ProductClass));
    to->set_volumemultiple(data->VolumeMultiple);
    to->set_pricetick(data->PriceTick);

    to->set_maxlimit(data->MaxLimitOrderVolume);
    to->set_minlimit(data->MinLimitOrderVolume);
    to->set_maxmarket(data->MaxMarketOrderVolume);
    to->set_minmartet(data->MinMarketOrderVolume);
}

/*
    // 代码相关
    string symbol = 1;          // 合约代码
    string exchange = 2;        // 交易所代码

    // 成交数据
    string actionDate = 3;      // 日期 20151009
    string tickTime = 4;        // 时间 11:20:56.500
    double lastPrice = 5;       // 最新成交价
    int32 volume = 6;           // 今天总成交量
    double openInterest = 7;    // 今天总持仓量
    int32 lastVolume = 8;       // 本笔成交量，无法计算时用1代替

    // 常规行情
    double openPrice = 9;       // 今日开盘价
    double highPrice = 10;      // 今日最高价
    double lowPrice = 11;       // 今日最低价
    double preClosePrice = 12;  // 昨日收盘价
    double upperLimit = 13;     // 涨停价
    double lowerLimit = 14;     // 跌停价

    // x档行情
    double bidPrice1 = 15;      // 买价
    double askPrice1 = 16;      // 卖价
    int32 bidVolume1 = 17;      // 买量
    int32 askVolume1 = 18;      // 卖量
*/

void translateTick(void* from, void* preFrom, BfTickData* to)
{
    auto data = (CThostFtdcDepthMarketDataField*)from;

    to->set_symbol(data->InstrumentID);
    to->set_exchange(data->ExchangeID);

    to->set_actiondate(data->ActionDay);
    QString tickTime = QString().sprintf("%s.%03d", data->UpdateTime, data->UpdateMillisec);
    to->set_ticktime(tickTime.toStdString());
    to->set_lastprice(data->LastPrice);
    to->set_volume(data->Volume);
    to->set_openinterest(data->OpenInterest);
    to->set_lastvolume(preFrom ? getVolumeFromTick(from) - getVolumeFromTick(preFrom) : 1);

    to->set_openprice(data->OpenPrice);
    to->set_highprice(data->HighestPrice);
    to->set_lowprice(data->LowestPrice);
    to->set_precloseprice(data->PreClosePrice);
    to->set_upperlimit(data->UpperLimitPrice);
    to->set_lowerlimit(data->LowerLimitPrice);

    to->set_bidprice1(data->BidPrice1);
    to->set_askprice1(data->AskPrice1);
    to->set_bidvolume1(data->BidVolume1);
    to->set_askvolume1(data->AskVolume1);
}

int getVolumeFromTick(void* tick)
{
    auto data = (CThostFtdcDepthMarketDataField*)tick;
    return data->Volume;
}

void translateBfOrderId(QString bfOrderId, int& frontId, int& sessionId, int& orderRef)
{
    orderRef = frontId = sessionId = 0;
    auto ids = bfOrderId.split(".");
    if (ids.length() == 3) {
        frontId = ids.at(0).toInt();
        sessionId = ids.at(1).toInt();
        orderRef = ids.at(2).toInt();
    }
}

QString formatBfOrderId(int frontId, int sessionId, int orderRef)
{
    return QString().sprintf("%d.%d.%d", frontId, sessionId, orderRef);
}
}
