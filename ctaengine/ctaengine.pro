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
    ui/tablewidget_helper.cpp

HEADERS  += ui/mainwindow.h \
    profile.h \
    servicemgr.h \
    ctpmgr.h \
    pushservice.h \
    rpcservice.h \
    dbservice.h \
    ui/tablewidget_helper.h

FORMS    += ui/mainwindow.ui

include(../base/base.pri)
include(../third_party/breakpad.pri)
include(../third_party/mhook.pri)
include(../third_party/ctp.pri)
include(../third_party/leveldb.pri)

RESOURCES += \
    systray.qrc
