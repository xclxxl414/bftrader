#ifndef ZTALIB_H
#define ZTALIB_H

#ifdef ZTALIB_DLL
#define TA_EXPORT extern "C" __declspec(dllexport)
#include "ta_func.h"
#include "ta_abstract.h"
#else
#define TA_EXPORT extern "C" __declspec(dllimport)
#define TA_RetCode int
#define TA_MAType int
#endif

TA_EXPORT TA_RetCode Z_Initialize();

TA_EXPORT TA_RetCode Z_Shutdown();

TA_EXPORT TA_RetCode Z_Acos(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Ad(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, const double *inVolume, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Add(int startIdx, int endIdx, const double *inReal0, const double *inReal1, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_AdOsc(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, const double *inVolume, int optInFastPeriod, /* From 2 to 100000 */ int optInSlowPeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Adx(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Adxr(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Apo(int startIdx, int endIdx, const double *inReal, int optInFastPeriod, /* From 2 to 100000 */ int optInSlowPeriod, /* From 2 to 100000 */ TA_MAType optInMAType, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_AroOn(int startIdx, int endIdx, const double *inHigh, const double *inLow, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outAroonDown, double *outAroonUp);

TA_EXPORT TA_RetCode Z_AroOnOsc(int startIdx, int endIdx, const double *inHigh, const double *inLow, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_ASin(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Atan(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Atr(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_AvgPrice(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_BBands(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ double optInNbDevUp, /* From TA_REAL_MIN to TA_REAL_MAX */ double optInNbDevDn, /* From TA_REAL_MIN to TA_REAL_MAX */ TA_MAType optInMAType, int *outBegIdx, int *outNBElement, double *outRealUpperBand, double *outRealMiddleBand, double *outRealLowerBand);

TA_EXPORT TA_RetCode Z_Beta(int startIdx, int endIdx, const double *inReal0, const double *inReal1, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Bop(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Cci(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Cdl2Crows(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Cdl3BlackCrows(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Cdl3InSide(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Cdl3LineStrike(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Cdl3OutSide(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Cdl3StarsInSouth(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Cdl3WhiteSoldiers(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlAbandOnedBaBy(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlAdvanceBlock(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlBeltHold(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlBreakaway(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlCloSingMarubozu(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlCOncealBaBySwall(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlCounterattack(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlDarkCloudCover(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlDoji(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlDojiStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlDragOnflyDoji(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlEngulfing(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlEveningDojiStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlEveningStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlGapSidesideWhite(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlGravestOneDoji(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHammer(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHangingMan(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHarami(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHaramiCross(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHighWave(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHikkake(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHikkakeMod(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlHoMingPigeOn(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlIdentical3Crows(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlinNeck(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlinvertedHammer(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlKicking(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlKickingByLength(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlLadderBottom(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlLOngLeggedDoji(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlLOngLine(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlMarubozu(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlMatchingLow(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlMatHold(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlMorningDojiStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlMorningStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, double optInPenetration, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlOnNeck(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlPiercing(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlRickshawMan(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlRiseFall3Methods(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlSeparatingLines(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlShootingStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlShortLine(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlSpinningTop(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlStalledPattern(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlStickSandwich(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlTakuri(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlTasukiGap(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlThrusting(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdltriStar(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlUnique3River(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlupSideGap2Crows(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_CdlxSideGap3Methods(int startIdx, int endIdx, const double *inOpen, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Ceil(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Cmo(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Correl(int startIdx, int endIdx, const double *inReal0, const double *inReal1, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Cos(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Cosh(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Dema(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Div(int startIdx, int endIdx, const double *inReal0, const double *inReal1, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Dx(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Ema(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Exp(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Floor(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_HtDcPeriod(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_HtDcPhase(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_HtPhasor(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outInPhase, double *outQuadrature);

TA_EXPORT TA_RetCode Z_HtSine(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outSine, double *outLeadSine);

TA_EXPORT TA_RetCode Z_HtTrendLine(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_HtTrendMode(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_Kama(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_LinearReg(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_LinearRegAngle(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_LinearRegIntercept(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_LinearRegSlope(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Ln(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Log10(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Ma(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ TA_MAType optInMAType, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Macd(int startIdx, int endIdx, const double *inReal, int optInFastPeriod, /* From 2 to 100000 */ int optInSlowPeriod, /* From 2 to 100000 */ int optInSignalPeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outMACD, double *outMACDSignal, double *outMACDHist);

TA_EXPORT TA_RetCode Z_MacdExt(int startIdx, int endIdx, const double *inReal, int optInFastPeriod, /* From 2 to 100000 */ TA_MAType optInFastMAType, int optInSlowPeriod, /* From 2 to 100000 */ TA_MAType optInSlowMAType, int optInSignalPeriod, /* From 1 to 100000 */ TA_MAType optInSignalMAType, int *outBegIdx, int *outNBElement, double *outMACD, double *outMACDSignal, double *outMACDHist);

TA_EXPORT TA_RetCode Z_MacdFix(int startIdx, int endIdx, const double *inReal, int optInSignalPeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outMACD, double *outMACDSignal, double *outMACDHist);

TA_EXPORT TA_RetCode Z_Mama(int startIdx, int endIdx, const double *inReal, double optInFastLimit, /* From 0.01 to 0.99 */ double optInSlowLimit, /* From 0.01 to 0.99 */ int *outBegIdx, int *outNBElement, double *outMAMA, double *outFAMA);

TA_EXPORT TA_RetCode Z_Mavp(int startIdx, int endIdx, const double *inReal, const double *inPeriods, int optInMinPeriod, /* From 2 to 100000 */ int optInMaxPeriod, /* From 2 to 100000 */ TA_MAType optInMAType, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Max(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_MaxIndex(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_MedPrice(int startIdx, int endIdx, const double *inHigh, const double *inLow, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Mfi(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, const double *inVolume, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_MidPoint(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_MidPrice(int startIdx, int endIdx, const double *inHigh, const double *inLow, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Min(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_MinIndex(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, int *outInteger);

TA_EXPORT TA_RetCode Z_MinMax(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outMin, double *outMax);

TA_EXPORT TA_RetCode Z_MinMaxIndex(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, int *outMinIdx, int *outMaxIdx);

TA_EXPORT TA_RetCode Z_MinusDi(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_MinusDm(int startIdx, int endIdx, const double *inHigh, const double *inLow, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Mom(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Mult(int startIdx, int endIdx, const double *inReal0, const double *inReal1, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Natr(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Obv(int startIdx, int endIdx, const double *inReal, const double *inVolume, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_PlusDi(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_PlusDm(int startIdx, int endIdx, const double *inHigh, const double *inLow, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Ppo(int startIdx, int endIdx, const double *inReal, int optInFastPeriod, /* From 2 to 100000 */ int optInSlowPeriod, /* From 2 to 100000 */ TA_MAType optInMAType, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Roc(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Rocp(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Rocr(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Rocr100(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Rsi(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Sar(int startIdx, int endIdx, const double *inHigh, const double *inLow, double optInAcceleration, /* From 0 to TA_REAL_MAX */ double optInMaximum, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_SarExt(int startIdx, int endIdx, const double *inHigh, const double *inLow, double optInStartValue, /* From TA_REAL_MIN to TA_REAL_MAX */ double optInOffsetOnReverse, /* From 0 to TA_REAL_MAX */ double optInAccelerationInitLong, /* From 0 to TA_REAL_MAX */ double optInAccelerationLong, /* From 0 to TA_REAL_MAX */ double optInAccelerationMaxLong, /* From 0 to TA_REAL_MAX */ double optInAccelerationInitShort, /* From 0 to TA_REAL_MAX */ double optInAccelerationShort, /* From 0 to TA_REAL_MAX */ double optInAccelerationMaxShort, /* From 0 to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Sin(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Sinh(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Sma(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Sqrt(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_StdDev(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ double optInNbDev, /* From TA_REAL_MIN to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Stoch(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInFastK_Period, /* From 1 to 100000 */ int optInSlowK_Period, /* From 1 to 100000 */ TA_MAType optInSlowK_MAType, int optInSlowD_Period, /* From 1 to 100000 */ TA_MAType optInSlowD_MAType, int *outBegIdx, int *outNBElement, double *outSlowK, double *outSlowD);

TA_EXPORT TA_RetCode Z_Stochf(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInFastK_Period, /* From 1 to 100000 */ int optInFastD_Period, /* From 1 to 100000 */ TA_MAType optInFastD_MAType, int *outBegIdx, int *outNBElement, double *outFastK, double *outFastD);

TA_EXPORT TA_RetCode Z_StochRsi(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int optInFastK_Period, /* From 1 to 100000 */ int optInFastD_Period, /* From 1 to 100000 */ TA_MAType optInFastD_MAType, int *outBegIdx, int *outNBElement, double *outFastK, double *outFastD);

TA_EXPORT TA_RetCode Z_Sub(int startIdx, int endIdx, const double *inReal0, const double *inReal1, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Sum(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_T3(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ double optInVFactor, /* From 0 to 1 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Tan(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Tanh(int startIdx, int endIdx, const double *inReal, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Tema(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Trange(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Trima(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Trix(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Tsf(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_TypPrice(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_UltOsc(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod1, /* From 1 to 100000 */ int optInTimePeriod2, /* From 1 to 100000 */ int optInTimePeriod3, /* From 1 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Var(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 1 to 100000 */ double optInNbDev, /* From TA_REAL_MIN to TA_REAL_MAX */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_WclPrice(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Willr(int startIdx, int endIdx, const double *inHigh, const double *inLow, const double *inClose, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

TA_EXPORT TA_RetCode Z_Wma(int startIdx, int endIdx, const double *inReal, int optInTimePeriod, /* From 2 to 100000 */ int *outBegIdx, int *outNBElement, double *outReal);

#endif // ZTALIB_H
