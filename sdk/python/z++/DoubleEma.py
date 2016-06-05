# coding=utf-8

import time
import random

from bfgateway_pb2 import *
from bfdatafeed_pb2 import *
from google.protobuf.any_pb2 import *

from grpc.beta import implementations
from grpc.beta import interfaces

_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1
_CLIENT_ID = "dualcross"
_MT = [("clientid", _CLIENT_ID)]

_PING_TYPE = BfPingData().DESCRIPTOR
_ACCOUNT_TYPE = BfAccountData().DESCRIPTOR
_POSITION_TYPE = BfPositionData().DESCRIPTOR
_TICK_TYPE = BfTickData().DESCRIPTOR
_TRADE_TYPE = BfTradeData().DESCRIPTOR
_ORDER_TYPE = BfOrderData().DESCRIPTOR
_LOG_TYPE = BfLogData().DESCRIPTOR
_ERROR_TYPE = BfErrorData().DESCRIPTOR
_NOTIFICATION_TYPE = BfNotificationData().DESCRIPTOR

EMPTY_STRING = ''
EMPTY_FLOAT = 0.0
EMPTY_INT = 0

SYMBOL = 'rb1609'
EXCHANGE = 'SHFE'


class DualCross(object):
    # 策略参数
    fastK = 0.9  # 快速EMA参数
    slowK = 0.1  # 慢速EMA参数
    initDays = 10  # 初始化数据所用的天数
    theMaxCounter = 10  # 第二个bar等待多久去取datafeed的数据

    # 策略变量
    bar = None
    barMinute = EMPTY_INT
    theFirstBarMinute = EMPTY_INT
    theCounter = EMPTY_INT

    orderData = {}  # 订单
    pos = {'long': EMPTY_INT, 'short': EMPTY_INT}  # 仓位

    # 是否是第一个tick
    theFirstTick = True

    fastMa = []  # 快速EMA均线数组
    fastMa0 = EMPTY_FLOAT  # 当前最新的快速EMA
    fastMa1 = EMPTY_FLOAT  # 上一根的快速EMA

    slowMa = []  # 与上面相同
    slowMa0 = EMPTY_FLOAT
    slowMa1 = EMPTY_FLOAT

    def __init__(self):
        print "init dualcross"

        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = beta_create_BfGatewayService_stub(self.gateway_channel)
        self.datafeed_channel = implementations.insecure_channel('localhost', 50052)
        self.datafeed = beta_create_BfDatafeedService_stub(self.datafeed_channel)
        self.connectivity = interfaces.ChannelConnectivity.IDLE

    def update(self, connectivity):
        '''C:\projects\grpc\src\python\grpcio\tests\unit\beta\_connectivity_channel_test.py'''
        print connectivity
        self.connectivity = connectivity

    def subscribe(self):
        self.gateway_channel.subscribe(self.update, try_to_connect=True)

    def unsubscribe(self):
        self.gateway_channel.unsubscribe(self.update)

    def OnInit(self):
        # load tick
        print "OnInit-->loadBar"

    def OnTradeWillBegin(self, request):
        print "OnTradeWillBegin"
        print request

    def OnGotContracts(self, request):
        print "OnGotContracts"
        print request

        # GetContract
        for i in range(1, 1000):
            req = BfGetContractReq(index=i, subscribled=True)
            resp = self.gateway.GetContract(req, _TIMEOUT_SECONDS, metadata=_MT)
            if (resp.symbol):
                print resp
            else:
                break

        # QueryPosition
        req = BfVoid()
        resp = self.gateway.QueryPosition(req, _TIMEOUT_SECONDS, metadata=_MT)
        print resp

        # QueryAccount
        req = BfVoid()
        resp = self.gateway.QueryAccount(req, _TIMEOUT_SECONDS, metadata=_MT)
        print resp

    def OnPing(self, request, ):
        print "OnPing"
        print request

        # def OnTick(self, request):
        #     print "OnTick"
        #     print request

    def OnTick(self, tick):
        """收到行情TICK推送（必须由用户继承实现）"""
        print 'OnTick'

        tickHour = tick.tickTime.split(':')[0]
        tickMinute = tick.tickTime.split(':')[1]
        barMinute = int(tickHour) * 60 + int(tickMinute)  # 暂时不考虑隔夜交易
        # 收到的第一个tick
        if self.theFirstTick:
            self.theFirstTick = False
            # 确定第一个barMinute
            self.theFirstBarMinute = barMinute
        elif barMinute == self.theFirstBarMinute:
            # 在第一个minute内的数据从datarecorder里面获取
            return
        else:
            if ((barMinute - self.theFirstBarMinute) == 1):
                # 在第二分钟，由于datarecorder未必处理完前一分钟的bar数据，故延迟theMaxCounter取数
                self.theCounter += 1
                if self.theCounter == self.theMaxCounter:
                    req = BfGetBarReq(symbol=SYMBOL, exchange=EXCHANGE, period=PERIOD_M1, toDate=tick.actionDate,
                                      toTime=str(self.theFirstBarMinute / 60)
                                             + ":"
                                             + str(self.theFirstBarMinute % 60)
                                             + ":00.000",
                                      count=1)
                    responses = self.datafeed.GetBar(req, timeout=_ONE_DAY_IN_SECONDS, metadata=_MT)
                    for resp in responses:
                        print resp
                        self.OnBar(resp)

            # 计算K线
            if barMinute != self.barMinute:
                if self.bar:
                    self.OnBar(self.bar)

                bar = BfBarData()
                bar.symbol = tick.symbol
                bar.exchange = tick.exchange
                bar.period = PERIOD_M1

                bar.openPrice = tick.lastPrice
                bar.highPrice = tick.lastPrice
                bar.lowPrice = tick.lastPrice
                bar.closePrice = tick.lastPrice

                bar.actionDate = tick.actionDate
                bar.barTime = tick.tickTime

                # 实盘中用不到的数据可以选择不算，从而加快速度
                # bar.volume = tick.volume
                # bar.openInterest = tick.openInterest

                self.bar = bar  # 这种写法为了减少一层访问，加快速度
                self.barMinute = barMinute  # 更新当前的分钟

            else:  # 否则继续累加新的K线
                bar = self.bar  # 写法同样为了加快速度

                bar.highPrice = max(bar.highPrice, tick.lastPrice)
                bar.lowPrice = min(bar.lowPrice, tick.lastPrice)
                bar.closePrice = tick.lastPrice

    # ----------------------------------------------------------------------
    def OnBar(self, bar):
        """收到Bar推送（必须由用户继承实现）"""
        # 计算快慢均线
        if not self.fastMa0:
            self.fastMa0 = bar.closePrice
            self.fastMa.append(self.fastMa0)
        else:
            self.fastMa1 = self.fastMa0
            self.fastMa0 = bar.closePrice * self.fastK + self.fastMa0 * (1 - self.fastK)
            self.fastMa.append(self.fastMa0)

        if not self.slowMa0:
            self.slowMa0 = bar.closePrice
            self.slowMa.append(self.slowMa0)
        else:
            self.slowMa1 = self.slowMa0
            self.slowMa0 = bar.closePrice * self.slowK + self.slowMa0 * (1 - self.slowK)
            self.slowMa.append(self.slowMa0)

        # 判断买卖
        crossOver = self.fastMa0 > self.slowMa0 and self.fastMa1 < self.slowMa1  # 金叉上穿
        crossBelow = self.fastMa0 < self.slowMa0 and self.fastMa1 > self.slowMa1  # 死叉下穿

        # 金叉和死叉的条件是互斥
        # 所有的委托均以K线收盘价委托（这里有一个实盘中无法成交的风险，考虑添加对模拟市价单类型的支持）
        # self.gateway.QueryPosition()
        if crossOver:
            req1 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=bar.closePrice, volume=self.pos['short'],
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_LONG, offset=OFFSET_CLOSE)
            req2 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=bar.closePrice, volume=1,
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_LONG, offset=OFFSET_OPEN)
            self.gateway.SendOrder(req1)
            self.gateway.SendOrder(req2)  # 死叉和金叉相反
        elif crossBelow:
            req1 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=bar.closePrice, volume=self.pos['long'],
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_SHORT, offset=OFFSET_CLOSE)
            req2 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=bar.closePrice, volume=1,
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_SHORT, offset=OFFSET_OPEN)
            self.gateway.SendOrder(req1, _TIMEOUT_SECONDS, metadata=_MT)
            self.gateway.SendOrder(req2, _TIMEOUT_SECONDS, metadata=_MT)

    def OnError(self, request):
        print "OnError"
        print request

    def OnLog(self, request):
        print "OnLog"
        print request

    def OnTrade(self, tradeData):
        print "OnTrade"
        print tradeData
        if tradeData.symbol == SYMBOL:
            if tradeData.direction == DIRECTION_LONG and tradeData.offset == OFFSET_OPEN:
                self.pos['long'] += 1
            elif tradeData.direction == DIRECTION_SHORT and tradeData.offset == OFFSET_OPEN:
                self.pos['short'] += 1
            elif tradeData.direction == DIRECTION_LONG and tradeData.offset == OFFSET_CLOSE:
                self.pos['short'] -= 1
            elif tradeData.direction == DIRECTION_SHORT and tradeData.offset == OFFSET_CLOSE:
                self.pos['long'] -= 1

    def OnOrder(self, orderData):
        print "OnOrder"
        print orderData
        if orderData.symbol == SYMBOL:
            self.orderData[orderData.bfOrderId] = orderData

    def OnPosition(self, request):
        print "OnPosition"
        self.pos = request
        print request

    def OnAccount(self, request):
        print "OnAccount"
        print request

    def OnStop(self):
        print 'OnStop'
        for key in self.orderData:
            if self.orderData[key].status == STATUS_NOTTRADED or self.orderData[key].status == STATUS_PARTTRADED:
                req = BfCancelOrderReq(symbol=SYMBOL, exchange=EXCHANGE, bfOrderId=self.orderData[key].bfOrderId)
                self.gateway.SendOrder(req, _TIMEOUT_SECONDS, metadata=_MT)


def dispatchPush(client, resp):
    if resp.Is(_TICK_TYPE):
        resp_data = BfTickData()
        resp.Unpack(resp_data)
        client.OnTick(resp_data)
    elif resp.Is(_PING_TYPE):
        resp_data = BfPingData()
        resp.Unpack(resp_data)
        client.OnPing(resp_data)
    elif resp.Is(_ACCOUNT_TYPE):
        resp_data = BfAccountData()
        resp.Unpack(resp_data)
        client.OnAccount(resp_data)
    elif resp.Is(_POSITION_TYPE):
        resp_data = BfPositionData()
        resp.Unpack(resp_data)
        client.OnPosition(resp_data)
    elif resp.Is(_TRADE_TYPE):
        resp_data = BfTradeData()
        resp.Unpack(resp_data)
        client.OnTrade(resp_data)
    elif resp.Is(_ORDER_TYPE):
        resp_data = BfOrderData()
        resp.Unpack(resp_data)
        client.OnOrder(resp_data)
    elif resp.Is(_LOG_TYPE):
        resp_data = BfLogData()
        resp.Unpack(resp_data)
        client.OnLog(resp_data)
    elif resp.Is(_ERROR_TYPE):
        resp_data = BfErrorData()
        resp.Unpack(resp_data)
        client.OnError(resp_data)
    elif resp.Is(_NOTIFICATION_TYPE):
        resp_data = BfNotificationData()
        resp.Unpack(resp_data)
        if resp_data.type == NOTIFICATION_GOTCONTRACTS:
            client.OnGotContracts(resp_data)
        elif resp_data.type == NOTIFICATION_TRADEWILLBEGIN:
            client.OnTradeWillBegin(resp_data)
        else:
            print "invliad notification type"
    else:
        print "invalid push type"


def connect(client):
    print "connect gateway"
    req = BfConnectReq(clientId=_CLIENT_ID, tickHandler=True, tradeHandler=True, logHandler=True, symbol=SYMBOL,
                       exchange=EXCHANGE)
    responses = client.gateway.Connect(req, timeout=_ONE_DAY_IN_SECONDS)
    for resp in responses:
        dispatchPush(client, resp)
    print "connect quit"


def disconnect(client):
    print "disconnect gateway"
    req = BfVoid()
    resp = client.gateway.Disconnect(req, _TIMEOUT_SECONDS, metadata=_MT)


def tryconnect(client):
    '''subscribe dont tryconnect after server shutdown. so unsubscrible and subscrible again'''
    print "sleep 5s,try reconnect..."
    time.sleep(_TIMEOUT_SECONDS)
    client.unsubscribe()
    time.sleep(_TIMEOUT_SECONDS)
    client.subscribe()
    time.sleep(_TIMEOUT_SECONDS)
    time.sleep(_TIMEOUT_SECONDS)
    time.sleep(_TIMEOUT_SECONDS)


def run():
    print "start dualcross"
    client = DualCross()
    client.subscribe()
    client.OnInit()

    try:
        while True:
            if client.connectivity == interfaces.ChannelConnectivity.READY:
                connect(client)
            tryconnect(client)
    except KeyboardInterrupt:
        print "ctrl+c"

    if client.connectivity == interfaces.ChannelConnectivity.READY:
        client.OnStop()
        disconnect(client)

    print "stop dualcross"
    client.unsubscribe()


if __name__ == '__main__':
    run()
