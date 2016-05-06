# coding=utf-8

import time
import random

from bftrader_pb2 import *
from bfkv_pb2 import *

from grpc.beta import implementations

_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 2

def generate_messages():
    msg = BfPingData(message="ping")
    for i in range(1,10):
        print i
        yield msg
        time.sleep(random.uniform(0.5, 1))
        
def PingStreamCS(kvservice):
    responses = kvservice.PingStreamCS(generate_messages(), _ONE_DAY_IN_SECONDS)
    for resp in responses:
        print resp

def PingStreamC(kvservice):
    resp = kvservice.PingStreamC(generate_messages(), _ONE_DAY_IN_SECONDS)
    print resp

def PingStreamS(kvservice):
    req = BfPingData(message="ping")
    responses = kvservice.PingStreamS(req, _ONE_DAY_IN_SECONDS)
    for resp in responses:
        print resp
        
def Ping(kvservice):
    req = BfPingData(message="ping")
    resp = kvservice.Ping(req,_TIMEOUT_SECONDS)
    print resp
    
def run():
    print "connect kvservice"
    kvservice_channel = implementations.insecure_channel('localhost', 50059)
    kvservice = beta_create_BfKvService_stub(kvservice_channel)    

    print "===Ping==="
    Ping(kvservice)
    print "===PingStreamCS==="
    PingStreamCS(kvservice)
    print "===PingStreamS==="
    PingStreamS(kvservice)
    print "===PingStreamC==="
    PingStreamC(kvservice)
    
    print "quit"
    
if __name__ == '__main__':
    run()
