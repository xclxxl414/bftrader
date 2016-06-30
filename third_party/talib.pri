TALIB_DIR = $$PWD/talib

# include
INCLUDEPATH += $$TALIB_DIR/include

# lib
TALIB_LIB_DIR = $$TALIB_DIR/lib
DEBUG_NAME_POST = ""
CONFIG(debug,debug|release) {
    DEBUG_NAME_POST = "d"
} else {
}

LIBS += $$TALIB_LIB_DIR/talib$${DEBUG_NAME_POST}.lib

HEADERS += \
    $$TALIB_DIR/include/*.h
