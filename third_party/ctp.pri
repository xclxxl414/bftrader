CTP_DIR = $$PWD/ctp

INCLUDEPATH += \
    $$CTP_DIR \
    $$CTP_DIR/include

SOURCES += \
    $$CTP_DIR/mdapi_proxy.cpp \
    $$CTP_DIR/tdapi_proxy.cpp

HEADERS += \
    $$CTP_DIR/include/*.h

