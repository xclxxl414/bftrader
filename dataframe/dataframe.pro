#-------------------------------------------------
#
# Project created by QtCreator 2016-04-24T13:02:55
#
#-------------------------------------------------

QT       -= core gui
QT += core
TEMPLATE = lib
DEFINES += DATAFRAME_LIBRARY

include(../bftrader.pri)

SOURCES += dataframe.cpp

HEADERS += dataframe.h\
        dataframe_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
