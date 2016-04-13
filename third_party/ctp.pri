SOURCE_DIR = $$PWD/ctp
INCLUDEPATH += \
    $$SOURCE_DIR

SOURCES += \
    $$SOURCE_DIR/mdapi_proxy.cpp \
    $$SOURCE_DIR/tdapi_proxy.cpp

VNSDK_CTP_DIR = c:/vnsdk/ctp/include
INCLUDEPATH += $$VNSDK_CTP_DIR
HEADERS += \
    $$VNSDK_CTP_DIR/*.h

