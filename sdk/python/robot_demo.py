# coding=utf-8

import time

import bftrader_pb2
import bfrobot_pb2
import bfgateway_pb2
import bfdatafeed_pb2

from grpc.beta import implementations

_BF_VOID = bftrader_pb2.BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1

class Robot(bfrobot_pb2.BetaBfRobotServiceServicer):
    def __init__(self):
        print "init robot"
        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = bfgateway_pb2.beta_create_BfGatewayService_stub(self.gateway_channel)
        self.datafeed_channel = implementations.insecure_channel('localhost', 50052)
        self.datafeed = bfdatafeed_pb2.beta_create_BfDatafeedService_stub(self.datafeed_channel)
        self._service = bfrobot_pb2.beta_create_BfRobotService_server(self)
        self._service.add_insecure_port('[::]:50053')
        
    def start(self):
        self._service.start()
    
    def stop(self):
        self._service.stop(0)
        
    def OnExchangeOpened(self, request, context):
        print "OnExchangeOpened"
        return _BF_VOID
        
    def OnTick(self, request, context):
        print "OnTick"
        return _BF_VOID
        
    def OnError(self, request, context):
        print "OnError"
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
    
    def OnExchangeClosed(self, request, context):
        print "OnExchangeClosed"
        return _BF_VOID
    
def run():
    print "start robot"
    robot = Robot()
    robot.start()

    print "connect gateway"
    bfconnectresp = robot.gateway.Connect(bftrader_pb2.BfConnectReq(robotId="demo",robotIp="localhost",robotPort=50053),_TIMEOUT_SECONDS)
    if bfconnectresp.exchangeOpened:
        mt = [("robotid","demo")]
        bfvoid = robot.gateway.Subscribe(bftrader_pb2.BfSubscribeReq(symbol="*",exchange="*"),_TIMEOUT_SECONDS,metadata=mt)
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        print "stop robot"
        robot.stop()

if __name__ == '__main__':
    run()
