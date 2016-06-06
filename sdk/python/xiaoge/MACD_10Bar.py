# coding=utf-8
import datetime as dt
import pandas as pd
import numpy as  np

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
_MT = [("clientid",_CLIENT_ID)]

_PING_TYPE = BfPingData().DESCRIPTOR
_ACCOUNT_TYPE = BfAccountData().DESCRIPTOR
_POSITION_TYPE = BfPositionData().DESCRIPTOR
_TICK_TYPE = BfTickData().DESCRIPTOR
_TRADE_TYPE = BfTradeData().DESCRIPTOR
_ORDER_TYPE = BfOrderData().DESCRIPTOR
_LOG_TYPE = BfLogData().DESCRIPTOR
_ERROR_TYPE = BfErrorData().DESCRIPTOR
_NOTIFICATION_TYPE = BfNotificationData().DESCRIPTOR

SYMBOL = 'rb1610'
EXCHANGE = 'SHFE'

class DualCross(object):
    def __init__(self):
        print "init dualcross"
        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = beta_create_BfGatewayService_stub(self.gateway_channel)
        self.datafeed_channel = implementations.insecure_channel('localhost',50052)
        self.datafeed = beta_create_BfDatafeedService_stub(self.datafeed_channel)
        self.connectivity = interfaces.ChannelConnectivity.IDLE

        # --------------------------------------------------
        self.FirstRun=1
        # 1Min Bar的时间戳
        dt_now = dt.datetime.now()
        dt_now = dt.datetime(2016,6,2,16,57)
        self.last_dt_bar = dt.datetime(dt_now.year, dt_now.month, dt_now.day, dt_now.hour, dt_now.minute, 0)\
                           - dt.timedelta(minutes=1)
        # 取出历史的200根Bar
        req = BfGetBarReq(symbol=SYMBOL, exchange=EXCHANGE, period=PERIOD_M1,
                          toDate=dt.datetime.strftime(self.last_dt_bar,"%Y%m%d"),
                          toTime=dt.datetime.strftime(self.last_dt_bar,"%H:%M:%S"),
                          count=200)
        # print req
        responses = self.datafeed.GetBar(req, timeout=_ONE_DAY_IN_SECONDS, metadata=_MT)
        closePrice_list = []
        barTime_list = []
        for resp in responses:
            closePrice_list = [resp.closePrice]+closePrice_list
            barTime_list = [resp.barTime] + barTime_list
        self.closePrice_ser = pd.Series(closePrice_list,index=barTime_list)
        print self.closePrice_ser
        # 初始Bar
        self.CntBar = resp
        self.last_bar = resp

        self.last_barTime = 0
        self.ToHold = 0
        self.LastHold = 0
        self.last_MACD = 0
        self.last_Cnt10 = 0

    def update(self,connectivity):
        '''C:\projects\grpc\src\python\grpcio\tests\unit\beta\_connectivity_channel_test.py'''
        print connectivity
        self.connectivity = connectivity
        
    def subscribe(self):
        self.gateway_channel.subscribe(self.update,try_to_connect=True)
    
    def unsubscribe(self):
        self.gateway_channel.unsubscribe(self.update)
        
    def OnInit(self):
        # load tick
        # tick-rb1609-SHFE-20160429-14:39:07.000
        print "OnInit-->loadTick"
        req = BfGetTickReq(symbol="rb1610",exchange="SHFE",toDate="20160429",toTime="14:39:07.000",count=100)
        responses = self.datafeed.GetTick(req,timeout=_ONE_DAY_IN_SECONDS,metadata=_MT)
        for resp in responses:
            print resp            
        
    def OnTradeWillBegin(self, request):
        print "OnTradeWillBegin"
        print request        

    def OnGotContracts(self, request):
        print "OnGotContracts"
        print request
        
        # GetContract
        for i in range(1,1000):
            req = BfGetContractReq(index=i,subscribled=True)
            resp = self.gateway.GetContract(req,_TIMEOUT_SECONDS,metadata=_MT)
            if (resp.symbol):
                print resp
            else:
                break
        
        # QueryPosition
        req = BfVoid()
        resp = self.gateway.QueryPosition(req,_TIMEOUT_SECONDS,metadata=_MT)
        print resp
        
        # QueryAccount
        req = BfVoid()
        resp = self.gateway.QueryAccount(req,_TIMEOUT_SECONDS,metadata=_MT)
        print resp
            
    def OnPing(self, request,):
        print "OnPing"
        print request

    def OnTick(self, request):
        # print "OnTick"
        # print request
        # df = self.datafeed.InsertTick(request,_TIMEOUT_SECONDS,metadata=_MT)
        # --------------------------------------------------
        # 当前时刻
        dt_now = dt.datetime.strptime( request.actionDate+" "+request.tickTime,"%Y%m%d %H:%M:%S.%f")
        print dt_now
        # 当前bar时间戳
        dt_bar = dt.datetime(dt_now.year,dt_now.month,dt_now.day,dt_now.hour,dt_now.minute,0)
        # 判断是否同一个Bar
        if (dt_bar == self.last_dt_bar):
            # 同一个Bar
            self.CntBar.volume = request.volume
            self.CntBar.openInterest = request.openInterest
            self.CntBar.highPrice = max(self.CntBar.highPrice, request.lastPrice)
            self.CntBar.lowPrice  = min(self.CntBar.lowPrice, request.lastPrice)
            self.CntBar.closePrice = request.lastPrice
        else:
            # 推送旧Bar
            self.CntBar.lastVolume = request.volume - self.last_bar.volume
            self.last_bar = self.CntBar
            # df = self.datafeed.InsertBar(self.CntBar, _TIMEOUT_SECONDS, metadata=_MT)
            print dt_bar
            # 建立新的CntBar
            self.last_dt_bar = dt_bar
            self.CntBar.symbol = request.symbol
            self.CntBar.exchange = request.exchange
            self.CntBar.period = PERIOD_M1
            # CntBar.datetime = dt_bar
            self.CntBar.actionDate = dt.datetime.strftime(dt_bar,"%Y%m%d")
            self.CntBar.barTime = dt.datetime.strftime(dt_bar,"%H:%M:%S")
            self.CntBar.openPrice = request.lastPrice
            self.CntBar.highPrice = request.lastPrice
            self.CntBar.lowPrice = request.lastPrice
            self.CntBar.closePrice = request.lastPrice
            self.CntBar.volume = request.volume
            self.CntBar.openInterest = request.openInterest
            self.CntBar.lastVolume = 1

            print "self.last_bar"
            print self.last_bar
            if self.FirstRun != 1:
                self.OnBar()
            self.FirstRun = 0

    def OnBar(self):
        print "OnBar"
        # print self.closePrice_ser
        closePrice_ser_2add = pd.Series(self.last_bar.closePrice,index=[self.last_bar.barTime])
        self.closePrice_ser = self.closePrice_ser.append(closePrice_ser_2add)

        MACD_n = [12, 26, 9]
        MACD_EM0 = self.closePrice_ser.ewm(span=MACD_n[0]).mean()
        MACD_EM1 = self.closePrice_ser.ewm(span=MACD_n[1]).mean()
        DIF = (MACD_EM0 - MACD_EM1).round(3)
        DEA = DIF.ewm(span=MACD_n[2]).mean()
        MACD = 2 * (DIF - DEA).round(3)
        print MACD

        if (self.last_MACD * MACD.ix[-1]<0):
            self.last_Cnt10 = 0
        if self.last_Cnt10 < 10:
            if MACD.ix[-1] > 0:
                self.ToHold = 1
            else:
                self.ToHold = -1
        else:
            self.ToHold = 0
        self.last_MACD = MACD.ix[-1]
        self.last_Cnt10 = self.last_Cnt10 + 1

        print self.ToHold
        self.OnToHold()

    def OnToHold(self):
        req1 = 0
        req2 = 0
        if (self.LastHold < 0) & (self.ToHold >= 0) :
            req1 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=self.last_bar.closePrice, volume=1,
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_LONG, offset=OFFSET_CLOSE)
        elif (self.LastHold > 0) & (self.ToHold <= 0) :
            req1 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=self.last_bar.closePrice, volume=1,
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_SHORT, offset=OFFSET_CLOSE)
        if (self.ToHold > 0):
            req2 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=self.last_bar.closePrice, volume=1,
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_LONG, offset=OFFSET_OPEN)
        elif (self.ToHold < 0):
            req2 = BfSendOrderReq(symbol=SYMBOL, exchange=EXCHANGE, price=self.last_bar.closePrice, volume=1,
                                  priceType=PRICETYPE_LIMITPRICE, direction=DIRECTION_SHORT, offset=OFFSET_OPEN)
        print req1
        print req2
        if req1 != 0:
            self.gateway.SendOrder(req1, _TIMEOUT_SECONDS, metadata=_MT)
        if req2 != 0:
            self.gateway.SendOrder(req2, _TIMEOUT_SECONDS, metadata=_MT)
        self.LastHold = self.ToHold

        
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
    
def dispatchPush(client,resp):
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
    req = BfConnectReq(clientId=_CLIENT_ID,tickHandler=True,tradeHandler=True,logHandler=True,symbol=SYMBOL,exchange=EXCHANGE)
    responses = client.gateway.Connect(req,timeout=_ONE_DAY_IN_SECONDS)
    for resp in responses:
        dispatchPush(client,resp)            
    print "connect quit"
    
def disconnect(client):
    print "disconnect gateway"
    req = BfVoid()
    resp = client.gateway.Disconnect(req,_TIMEOUT_SECONDS,metadata=_MT)
    
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
        disconnect(client)
    
    print "stop dualcross"
    client.unsubscribe()
    
if __name__ == '__main__':
    run()
