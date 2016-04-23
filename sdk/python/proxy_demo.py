# coding=utf-8

import time

import bftrader_pb2
import bfproxy_pb2
import bfgateway_pb2

from grpc.beta import implementations

_BF_VOID = bftrader_pb2.BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1
_MT = [("proxyid","demo")]
    
class Proxy(bfproxy_pb2.BetaBfProxyServiceServicer):
    def __init__(self):
        print "init proxy"
        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = bfgateway_pb2.beta_create_BfGatewayService_stub(self.gateway_channel)
        self._service = bfproxy_pb2.beta_create_BfProxyService_server(self)
        self._service.add_insecure_port('[::]:50053')
        
    def start(self):
        self._service.start()
    
    def stop(self):
        self._service.stop(0)
        
    def OnTradeWillBegin(self, request, context):
        print "OnTradeWillBegin"
        return _BF_VOID
        
    def OnPing(self, request, context):
        print "OnPing"
        message = request.message
        return bftrader_pb2.BfPingData(message=request.message)

    def OnTick(self, request, context):
        print "OnTick"
        return _BF_VOID
        
    def OnError(self, request, context):
        print "OnError"
        return _BF_VOID
            
    def OnLog(self, request, context):
        print "OnLog"
        return _BF_VOID
    
    def OnTrade(self, request, context):
        print "OnTrade"
        return _BF_VOID
    
    def OnOrder(self, request, context):
        print "OnOrder"
        return _BF_VOID
            
    def OnPosition(self, request, context):
        print "OnPosition"
        return _BF_VOID

    def OnAccount(self, request, context):
        print "OnAccount"
        return _BF_VOID
    
def run():
    print "start proxy"
    proxy = Proxy()
    proxy.start()

    print "connect gateway"
    bfconnectresp = proxy.gateway.Connect(bftrader_pb2.BfConnectReq(proxyId="demo",proxyIp="localhost",proxyPort=50053),_TIMEOUT_SECONDS)
    if bfconnectresp.errorCode == 0:
        bfpingdata = proxy.gateway.Ping(bftrader_pb2.BfPingData(message="ping"),_TIMEOUT_SECONDS,metadata=_MT)
        print "ping=%s,pong=%s" % ("ping",bfpingdata.message)
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        print "stop proxy"
        bfvoid = proxy.gateway.Close(_BF_VOID,_TIMEOUT_SECONDS,metadata=_MT)
        proxy.stop()

if __name__ == '__main__':
    run()
