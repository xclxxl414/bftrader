TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

include (../../bftrader.pri)

SOURCES += main.cpp

include (../../third_party/ctp.pri)
