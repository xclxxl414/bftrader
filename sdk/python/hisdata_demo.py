# coding=utf-8

import time

import bftrader_pb2
import bfhisdata_pb2

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

class HisData(bfhisdata_pb2.BetaBfHisDataServiceServicer):
    def __init__(self):
        print "init hisdata"
    
    def InsertTickPack(self, request, context):
        print "InsertTickPack"

    def InsertBar(self, request, context):
        print "InsertBar"

    def GetTickPack(self, request, context):
        print "GetTickPack"

    def GetBar(self, request, context):
        print "GetBar"
    
    
def run():
    hisdata = bfhisdata_pb2.beta_create_BfHisDataService_server(HisData())
    hisdata.add_insecure_port('[::]:50052')
    hisdata.start()
    print "start hisdata"
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        print "stop hisdata"
        hisdata.stop(0)

if __name__ == '__main__':
    run()