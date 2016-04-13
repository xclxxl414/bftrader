TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

include (../../../bftrader.pri)

INCLUDEPATH += $$PWD/../cpp

# src
SOURCES += \
    greeter_server.cc \
    ../cpp/helloworld.grpc.pb.cc \
    ../cpp/helloworld.pb.cc

# header
HEADERS += \
    ../cpp/helloworld.grpc.pb.h \
    ../cpp/helloworld.pb.h

include (../../../third_party/grpc.pri)
