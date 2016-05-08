# coding=utf-8

import time
import random

from bftrader_pb2 import *
from bfgateway_pb2 import *
from google.protobuf.any_pb2 import *

from grpc.beta import implementations
from grpc.beta import interfaces

_BF_VOID = BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1
_MT = [("clientid","gatewayclient_mock")]
_PING_TYPE = BfPingData().DESCRIPTOR

class GatewayClient(object):
    def __init__(self):
        print "init GatewayClient"
        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = beta_create_BfGatewayService_stub(self.gateway_channel)
        self.connectivity = interfaces.ChannelConnectivity.IDLE
    
    def update(self,connectivity):
        '''C:\projects\grpc\src\python\grpcio\tests\unit\beta\_connectivity_channel_test.py'''
        print connectivity
        self.connectivity = connectivity
        
    def subscribe(self):
        self.gateway_channel.subscribe(self.update,try_to_connect=True)
        pass
    
    def unsubscribe(self):
        self.gateway_channel.unsubscribe(self.update)
        pass
        
    def OnTradeWillBegin(self, request, context):
        print "OnTradeWillBegin"
        print request        
        return _BF_VOID

    def OnGotContracts(self, request, context):
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
        
        return _BF_VOID
    
    def OnPing(self, request, context):
        print "OnPing"
        print request
        message = request.message
        return BfPingData(message=request.message)

    def OnTick(self, request, context):
        print "OnTick"
        print request
        return _BF_VOID
        
    def OnError(self, request, context):
        print "OnError"
        print request
        return _BF_VOID
            
    def OnLog(self, request, context):
        print "OnLog"
        print request
        return _BF_VOID
    
    def OnTrade(self, request, context):
        print "OnTrade"
        print request
        return _BF_VOID
    
    def OnOrder(self, request, context):
        print "OnOrder"
        print request
        return _BF_VOID
            
    def OnPosition(self, request, context):
        print "OnPosition"
        print request
        return _BF_VOID

    def OnAccount(self, request, context):
        print "OnAccount"
        print request
        return _BF_VOID
    
def connect(client):
    print "connect gateway"
    req = BfConnectReq(clientId="gatewayclient_mock",tickHandler=True,tradeHandler=True,logHandler=True,symbol="*",exchange="*")
    responses = client.gateway.Connect(req,timeout=_ONE_DAY_IN_SECONDS)
    for resp in responses:
        if resp.Is(_PING_TYPE):
            resp_data = BfPingData()
            resp.Unpack(resp_data)
            print resp_data        
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
    print "start GatewayClient"
    client = GatewayClient()
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
    
    print "stop GatewayClient"
    client.unsubscribe()
    
if __name__ == '__main__':
    run()
