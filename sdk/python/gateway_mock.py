# coding=utf-8

import time

from bftrader_pb2 import *
from bfgateway_pb2 import *
from bfproxy_pb2 import *

from grpc.beta import implementations

_BF_VOID = BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1

class Gateway(BetaBfGatewayServiceServicer):
    def __init__(self):
        print "init gateway"
        self.proxy_channel = ""
        self.proxy = ""
        
    def Connect(self, request, context):
        print "Connect"
        print request
        
        self.proxy_channel = implementations.insecure_channel(request.proxyIp, request.proxyPort)
        self.proxy = beta_create_BfProxyService_stub(self.proxy_channel)

        # OnTick
        req = BfTickData()
        resp = self.proxy.OnTick(req,_TIMEOUT_SECONDS)

        resp = BfConnectResp(errorCode = 0)
        return resp

    def Ping(self, request, context):
        print "Ping"
        mt =  context.invocation_metadata()
        for it in mt:
            print "metadata: %s:%s" % (it[0],it[1])
        print request
        
        resp = BfPingData(message=request.message)
        return resp

    def GetContract(self, request, context):
        print "GetContract"
        print request
        
        resp = BfContractData()
        return resp

    def SendOrder(self, request, context):
        print "SendOrder"
        print request
        
        resp = BfSendOrderResp()
        return resp

    def CancelOrder(self, request, context):
        print "CancelOrder"
        print request
        
        return _BF_VOID

    def QueryAccount(self, request, context):
        print "QueryAccount"
        print request
        
        return _BF_VOID

    def QueryPosition(self, request, context):
        print "QueryPosition"
        print resquest
        
        return _BF_VOID

    def Close(self, request, context):
        print "Close"
        print request
        
        return _BF_VOID
        
def run():
    gateway = beta_create_BfGatewayService_server(Gateway())
    gateway.add_insecure_port('[::]:50051')
    gateway.start()
    print "start gateway"
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        print "stop gateway"
        gateway.stop(0)

if __name__ == '__main__':
    run()
