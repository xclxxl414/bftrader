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
    gatewaymgr.cpp \
    ui/configdialog.cpp \
    ctamgr.cpp

HEADERS  += ui/mainwindow.h \
    profile.h \
    servicemgr.h \
    pushservice.h \
    rpcservice.h \
    dbservice.h \
    ui/tablewidget_helper.h \
    gatewaymgr.h \
    ui/configdialog.h \
    ctamgr.h

FORMS    += ui/mainwindow.ui \
    ui/configdialog.ui

include(../base/base.pri)
include(../sdk/sdk.pri)
include(../third_party/breakpad.pri)
include(../third_party/leveldb.pri)
include(../third_party/mhook.pri)
include(../third_party/grpc.pri)

RESOURCES += \
    systray.qrc
