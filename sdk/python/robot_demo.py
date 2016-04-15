# coding=utf-8

import time

import bftrader_pb2
import bfrobot_pb2
import bfgateway_pb2
import bfdatafeed_pb2

from grpc.beta import implementations

_BF_VOID = bftrader_pb2.BfVoid()
_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 30

class Robot(bfrobot_pb2.BetaBfRobotServiceServicer):
    def __init__(self):
        print "init robot"
        self.gateway_channel = implementations.insecure_channel('localhost', 50051)
        self.gateway = bfgateway_pb2.beta_create_BfGatewayService_stub(self.gateway_channel)
        self.datafeed_channel = implementations.insecure_channel('localhost', 50052)
        self.datafeed = bfdatafeed_pb2.beta_create_BfDatafeedService_stub(self.datafeed_channel)
        self._robot = bfrobot_pb2.beta_create_BfRobotService_server(self)
        self._robot.add_insecure_port('[::]:50053')
        
    def start(self):
        self._robot.start()
    
    def stop(self):
        self._robot.stop(0)
        
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
        
    def OnContract(self, request, context):
        print "OnContract"
        return _BF_VOID
    
    def OnPosition(self, request, context):
        print "OnPosition"
        return _BF_VOID

    def OnAccount(self, request, context):
        print "OnAccount"
        return _BF_VOID
    
    
def run():
    print "start robot"
    robot = Robot()
    robot.start()

    print "connect gateway"
    bferrordata = robot.gateway.Connect(bftrader_pb2.BfConnectReq(robotId="demo",endpoint=50053),_TIMEOUT_SECONDS)
    bfvoid = robot.gateway.Subscribe(bftrader_pb2.BfSubscribeReq(symbol="rb1610",exchange="SSE"),_TIMEOUT_SECONDS)
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        print "stop robot"
        robot.stop()

if __name__ == '__main__':
    run()
