SOURCE_DIR = $$PWD/mhook
INCLUDEPATH += \
    $$SOURCE_DIR

SOURCES += \
    $$SOURCE_DIR/disasm-lib/*.c \
    $$SOURCE_DIR/mhook-lib/*.cpp
    

HEADERS += \
    $$SOURCE_DIR/disasm-lib/*.h \
    $$SOURCE_DIR/mhook-lib/*.h