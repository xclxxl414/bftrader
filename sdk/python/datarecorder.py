# coding=utf-8

import time

from bftrader_pb2 import *
from bfproxy_pb2 import *
from bfgateway_pb2 import *
from bfdatafeed_pb2 import *

from grpc.beta import implementations

_BF_VOID = BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1
_MT = [("clientid","datarecorder")]
    
class DataRecorder(BetaBfProxyServiceServicer):
    def __init__(self):
        print "init datarecorder"
        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = beta_create_BfGatewayService_stub(self.gateway_channel)
        self.datafeed_channel = implementations.insecure_channel('localhost',50052)
        self.datafeed = beta_create_BfDatafeedService_stub(self.datafeed_channel)
        self._service = beta_create_BfProxyService_server(self)
        self._service.add_insecure_port('[::]:50060')
        
    def start(self):
        self._service.start()
    
    def stop(self):
        self._service.stop(0)
        
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
                df = self.datafeed.InsertContract(resp,_TIMEOUT_SECONDS,metadata=_MT)
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
        resp = self.datafeed.InsertTick(request,_TIMEOUT_SECONDS,metadata=_MT)
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
    
def run():
    print "start datarecorder"
    datarecorder = DataRecorder()
    datarecorder.start()

    print "ping datafeed"
    req = BfPingData(message="ping")
    resp = datarecorder.datafeed.Ping(req,_TIMEOUT_SECONDS,metadata=_MT)
    print "ping=(%s),pong=(%s)" % (req.message,resp.message)
    
    print "connect gateway"
    # Connect
    req = BfConnectReq(clientId="datarecorder",clientIp="localhost",clientPort=50060,tickHandler=True,tradeHandler=True,logHandler=True,symbol="*",exchange="*")
    resp = datarecorder.gateway.Connect(req,_TIMEOUT_SECONDS)
    if resp.errorCode == 0:
        # Ping
        req = BfPingData(message="ping")
        resp = datarecorder.gateway.Ping(req,_TIMEOUT_SECONDS,metadata=_MT)
        print "ping=(%s),pong=(%s)" % (req.message,resp.message)
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        # Disconnect
        req = BfVoid()
        resp = datarecorder.gateway.Disconnect(req,_TIMEOUT_SECONDS,metadata=_MT)

        print "stop datarecorder"
        datarecorder.stop()

if __name__ == '__main__':
    run()
