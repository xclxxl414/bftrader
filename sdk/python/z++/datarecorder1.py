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
_CLIENT_ID = "datarecorder"
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


class DataRecorder(object):
    bar = None
    barMinute = ''

    def __init__(self):
        print "init datarecorder"
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
                df = self.datafeed.InsertContract(resp, _TIMEOUT_SECONDS, metadata=_MT)
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

    def OnPing(self, request):
        print "OnPing"
        print request

    def OnTick(self, tick):
        print "OnTick"
        print tick
        df = self.datafeed.InsertTick(tick, _TIMEOUT_SECONDS, metadata=_MT)

        tickMinute = tick.tickTime.split(':')[1]

        if tickMinute != self.barMinute:
            if self.bar:
                self.datafeed.InsertBar(self.bar, _TIMEOUT_SECONDS, metadata=_MT)

            bar = BfBarData()
            bar.symbol = tick.symbol
            bar.exchange = tick.exchange
            bar.period = PERIOD_M1

            bar.openPrice = tick.lastPrice
            bar.highPrice = tick.lastPrice
            bar.lowPrice = tick.lastPrice
            bar.closePrice = tick.lastPrice

            bar.actionDate = tick.actionDate
            bar.barTime = tick.tickTime.split('.')[0]

            # 实盘中用不到的数据可以选择不算，从而加快速度
            # bar.volume = tick.volume
            # bar.openInterest = tick.openInterest

            self.bar = bar  # 这种写法为了减少一层访问，加快速度
            self.barMinute = tickMinute  # 更新当前的分钟

        else:  # 否则继续累加新的K线
            bar = self.bar  # 写法同样为了加快速度

            bar.highPrice = max(bar.highPrice, tick.lastPrice)
            bar.lowPrice = min(bar.lowPrice, tick.lastPrice)
            bar.closePrice = tick.lastPrice

    def OnError(self, request):
        print "OnError"
        print request

    def OnLog(self, request):
        print "OnLog"
        print request

    def OnTrade(self, request):
        print "OnTrade"
        print request

    def OnOrder(self, request):
        print "OnOrder"
        print request

    def OnPosition(self, request):
        print "OnPosition"
        print request

    def OnAccount(self, request):
        print "OnAccount"
        print request


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
    req = BfConnectReq(clientId=_CLIENT_ID, tickHandler=True, tradeHandler=True, logHandler=True, symbol="rb1609",
                       exchange="SHFE")
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
    print "start DataRecorder"
    client = DataRecorder()
    client.subscribe()

    try:
        while True:
            if client.connectivity == interfaces.ChannelConnectivity.READY:
                connect(client)
            tryconnect(client)
    except KeyboardInterrupt:
        print "ctrl+c"

    if client.connectivity == interfaces.ChannelConnectivity.READY:
        disconnect(client)

    print "stop DataRecorder"
    client.unsubscribe()


if __name__ == '__main__':
    run()
