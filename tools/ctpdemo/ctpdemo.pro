#-------------------------------------------------
#
# Project created by QtCreator 2015-10-13T13:26:03
#
#-------------------------------------------------

QT       += core gui widgets

TEMPLATE = app

include(../../bftrader.pri)

#DEFINES += USE_CTPJOIN

SOURCES += main.cpp\
    ui/mainwindow.cpp \
    ui/configdialog.cpp \
    ui/logindialog.cpp \
    ui/instrumentsform.cpp \
    ui/ringbufferform.cpp \
    servicemgr.cpp \
    profile.cpp \
    ctpcmd.cpp \
    mdsm.cpp \
    ringbuffer.cpp \
    tdsm.cpp \
    ctpcmdmgr.cpp \
    ctpmgr.cpp \
    datapump.cpp \
    dbservice.cpp \
    ctp_utils.cpp


HEADERS  += ui/mainwindow.h \
    ui/configdialog.h \
    ui/logindialog.h \
    ui/instrumentsform.h \
    ui/ringbufferform.h \
    servicemgr.h \
    profile.h \
    ctpcmd.h \
    mdsm.h \
    ringbuffer.h \
    tdsm.h \
    ctpcmdmgr.h \
    ctpmgr.h \
    datapump.h \
    dbservice.h \
    ctp_utils.h


FORMS    += ui/mainwindow.ui \
    ui/configdialog.ui \
    ui/logindialog.ui \
    ui/instrumentsform.ui \
    ui/ringbufferform.ui

include(../../third_party/ctp.pri)
include(../../third_party/leveldb.pri)
include(../../third_party/breakpad.pri)
include(../../third_party/mhook.pri)
include(../../utils/utils.pri)

RESOURCES += \
    systray.qrc

