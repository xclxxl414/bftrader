# coding=utf-8

import time
import random

from bfgateway_pb2 import *
from google.protobuf.any_pb2 import *

from grpc.beta import implementations

_BF_VOID = BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24

class Gateway(BetaBfGatewayServiceServicer):
    def __init__(self):
        print "init gateway"
        
    def Connect(self, request, context):
        print "Connect"
        print request
        
        resp_data = BfPingData(message="ping")
        resp = Any()
        resp.Pack(resp_data)
        for i in range(1,10):
            print i
            yield resp
            time.sleep(random.uniform(0.5, 1))

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

    def Disconnect(self, request, context):
        print "Disconnect"
        mt =  context.invocation_metadata()
        for it in mt:
            print "metadata: %s:%s" % (it[0],it[1])
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
