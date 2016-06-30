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
    profile.cpp \
    servicemgr.cpp \
    pushservice.cpp \
    rpcservice.cpp \
    dbservice.cpp \
    mdsm.cpp \
    tdsm.cpp \
    ui/mainwindow.cpp \
    ui/configdialog.cpp \
    ui/logindialog.cpp \
    ui/debugform.cpp \
    ui/positionform.cpp \
    ui/contractform.cpp \
    ui/finishedorderform.cpp \
    ui/tradeform.cpp \
    ui/tickform.cpp \
    ui/accountform.cpp \
    ui/tablewidget_helper.cpp \
    ui/workingorderform.cpp \
    ui/ringbufferform.cpp \
    gatewaymgr.cpp \
    ctputils.cpp

HEADERS  += ui/mainwindow.h \
    profile.h \
    servicemgr.h \
    pushservice.h \
    rpcservice.h \
    dbservice.h \
    mdsm.h \
    tdsm.h \
    ui/configdialog.h \
    ui/logindialog.h \
    ui/debugform.h \
    ui/positionform.h \
    ui/contractform.h \
    ui/finishedorderform.h \
    ui/tradeform.h \
    ui/tickform.h \
    ui/accountform.h \
    ui/tablewidget_helper.h \
    ui/workingorderform.h \
    ui/ringbufferform.h \ 
    gatewaymgr.h \
    ctputils.h

FORMS    += ui/mainwindow.ui \
    ui/configdialog.ui \
    ui/logindialog.ui \
    ui/debugform.ui \
    ui/positionform.ui \
    ui/contractform.ui \
    ui/finishedorderform.ui \
    ui/tradeform.ui \
    ui/tickform.ui \
    ui/accountform.ui \
    ui/workingorderform.ui \
    ui/ringbufferform.ui

include(../base/base.pri)
include(../sdk/sdk.pri)
include(../third_party/breakpad.pri)
include(../third_party/mhook.pri)
include(../third_party/ctp.pri)
include(../third_party/grpc.pri)

RESOURCES += \
    systray.qrc

DISTFILES += \
    design.qmodel \
    gatewaymgr.qmodel
