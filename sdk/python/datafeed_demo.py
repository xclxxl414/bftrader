# coding=utf-8

import time

import bftrader_pb2
import bfdatafeed_pb2

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

class Datafeed(bfdatafeed_pb2.BetaBfDatafeedServiceServicer):
    def __init__(self):
        print "init datafeed"
    
    def InsertTickPack(self, request, context):
        print "InsertTickPack"

    def InsertBar(self, request, context):
        print "InsertBar"

    def GetTickPack(self, request, context):
        print "GetTickPack"

    def GetBar(self, request, context):
        print "GetBar"
    
    
def run():
    datafeed = bfdatafeed_pb2.beta_create_BfDatafeedService_server(Datafeed())
    datafeed.add_insecure_port('[::]:50052')
    datafeed.start()
    print "start datafeed"
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        print "stop datafeed"
        datafeed.stop(0)

if __name__ == '__main__':
    run()