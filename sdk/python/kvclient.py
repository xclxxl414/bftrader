# coding=utf-8

import time
import random

from bfgateway_pb2 import *
from bfkv_pb2 import *
from google.protobuf.any_pb2 import *

from grpc.beta import implementations

_ONE_DAY_IN_SECONDS = 60 * 60 * 24
_TIMEOUT_SECONDS = 1
_PING_TYPE = BfPingData().DESCRIPTOR

def generate_messages():
    req_data = BfPingData(message="ping")
    req = Any()
    req.Pack(req_data)
    for i in range(1,10):
        print i
        yield req
        time.sleep(random.uniform(0.5, 1))
        
def PingStreamCS(kvservice):
    '''C:\projects\grpc\third_party\protobuf\python\google\protobuf\internal\message_test.py'''
    responses = kvservice.PingStreamCS(generate_messages(), timeout=_ONE_DAY_IN_SECONDS)
    for resp in responses:
        if resp.Is(_PING_TYPE):
            resp_data = BfPingData()
            resp.Unpack(resp_data)
            print resp_data

def PingStreamC(kvservice):
    resp = kvservice.PingStreamC(generate_messages(), timeout=_ONE_DAY_IN_SECONDS)
    if resp.Is(_PING_TYPE):
        resp_data = BfPingData()
        resp.Unpack(resp_data)
        print resp_data

def PingStreamS(kvservice):
    req_data = BfPingData(message="ping")
    req = Any()
    req.Pack(req_data)
    responses = kvservice.PingStreamS(req, timeout=_ONE_DAY_IN_SECONDS)
    for resp in responses:
        if resp.Is(_PING_TYPE):
            resp_data = BfPingData()
            resp.Unpack(resp_data)
            print resp_data
        
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
