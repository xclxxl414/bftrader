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
    pushservice.cpp \
    rpcservice.cpp \
    dbservice.cpp \
    ui/tablewidget_helper.cpp \
    ui/contractform.cpp \
    ui/tickform.cpp \
    ui/barform.cpp \
    ui/statform.cpp \
    ui/debugform.cpp \
    ui/errorform.cpp \
    ui/infoform.cpp \
    gatewaymgr.cpp \
    protoutils.cpp

HEADERS  += ui/mainwindow.h \
    profile.h \
    servicemgr.h \
    pushservice.h \
    rpcservice.h \
    dbservice.h \
    ui/tablewidget_helper.h \
    ui/contractform.h \
    ui/tickform.h \
    ui/barform.h \
    ui/statform.h \
    ui/debugform.h \
    ui/errorform.h \
    ui/infoform.h \
    gatewaymgr.h \
    protoutils.h

FORMS    += ui/mainwindow.ui \
    ui/contractform.ui \
    ui/tickform.ui \
    ui/barform.ui \
    ui/statform.ui \
    ui/debugform.ui \
    ui/errorform.ui \
    ui/infoform.ui

include(../base/base.pri)
include(../sdk/sdk.pri)
include(../third_party/breakpad.pri)
include(../third_party/mhook.pri)
include(../third_party/leveldb.pri)
include(../third_party/grpc.pri)
include(../third_party/qcustomplot.pri)

RESOURCES += \
    systray.qrc
