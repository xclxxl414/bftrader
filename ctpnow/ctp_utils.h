#ifndef CTPUTILS_H
#define CTPUTILS_H

#include "bftrader.pb.h"
#include <QString>

using namespace bftrader;

namespace CtpUtils {
char translatePriceType(BfPriceType priceType);
char translateOffset(BfOffset offset);
char translateDirection(BfDirection direction);

BfPriceType translatePriceType(char priceType);
BfOffset translateOffset(char offset);
BfDirection translateDirection(char direction);
BfDirection translatePosiDirection(char direction);
BfStatus translateStatus(char status);
BfProduct translateProduct(char product);

QString formatDirection(BfDirection direction);
QString formatOffset(BfOffset offset);
QString formatStatus(BfStatus status);
QString formatProduct(BfProduct product);

char* getExchangeFromContract(void* contract);
int getVolumeMultipleFromContract(void* contract);
void translateContract(void* from, BfContractData* to);

void translateTick(void* from, void* preFrom, BfTickData* to);
int getVolumeFromTick(void* tick);

void translateBfOrderId(QString bfOrderId, int& frontId, int& sessionId, int& orderRef);
QString formatBfOrderId(int frontId, int sessionId, int orderRef);
}
#endif // CTPUTILS_H
