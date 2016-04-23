# coding=utf-8

import time

import bftrader_pb2
import bfgateway_pb2
import bfproxy_pb2

from grpc.beta import implementations

_BF_VOID = bftrader_pb2.BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1

class Gateway(bfgateway_pb2.BetaBfGatewayServiceServicer):
    def __init__(self):
        print "init gateway"
        self.proxy_channel = ""
        self.proxy = ""
        
    def Connect(self, request, context):
        print "Connect"
        print request.proxyId,request.proxyIp,request.proxyPort
        self.proxy_channel = implementations.insecure_channel(request.proxyIp, request.proxyPort)
        self.proxy = bfproxy_pb2.beta_create_BfProxyService_stub(self.proxy_channel)
        bfvoid = self.proxy.OnTick(bftrader_pb2.BfTickData(),_TIMEOUT_SECONDS)        
        return bftrader_pb2.BfConnectResp(exchangeOpened = True)

    def Ping(self, request, context):
        print "Ping"
        mt =  context.invocation_metadata()
        for it in mt:
            print "metadata: %s:%s" % (it[0],it[1])
        print "Ping"

    def GetContract(self, request, context):
        print "GetContract"

    def SendOrder(self, request, context):
        print "SendOrder"

    def CancelOrder(self, request, context):
        print "CancelOrder"

    def QueryAccount(self, request, context):
        print "QueryAccount"

    def QueryPosition(self, request, context):
        print "QueryPosition"

    def Close(self, request, context):
        return _BF_VOID
        
def run():
    gateway = bfgateway_pb2.beta_create_BfGatewayService_server(Gateway())
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
