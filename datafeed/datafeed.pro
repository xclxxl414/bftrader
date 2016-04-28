#-------------------------------------------------
#
# Project created by QtCreator 2015-10-22T12:46:33
#
#-------------------------------------------------

QT       += core gui widgets
# QT += core_private

TEMPLATE = app

include(../bftrader.pri)

SOURCES += main.cpp\
    ui/mainwindow.cpp \
    profile.cpp \
    servicemgr.cpp \
    ctpmgr.cpp \
    pushservice.cpp \
    rpcservice.cpp \
    dbservice.cpp \
    ui/tablewidget_helper.cpp \
    ui/historycontractform.cpp \
    ui/historytickform.cpp \
    proto_utils.cpp

HEADERS  += ui/mainwindow.h \
    profile.h \
    servicemgr.h \
    ctpmgr.h \
    pushservice.h \
    rpcservice.h \
    dbservice.h \
    ui/tablewidget_helper.h \
    ui/historycontractform.h \
    ui/historytickform.h \
    proto_utils.h

FORMS    += ui/mainwindow.ui \
    ui/historycontractform.ui \
    ui/historytickform.ui

include(../base/base.pri)
include(../sdk/sdk.pri)
include(../third_party/breakpad.pri)
include(../third_party/mhook.pri)
include(../third_party/leveldb.pri)
include(../third_party/grpc.pri)
include(../third_party/qcustomplot.pri)

RESOURCES += \
    systray.qrc
