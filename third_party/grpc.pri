GRPC_DIR = $$PWD/grpc

# include
INCLUDEPATH += $$GRPC_DIR/include

# lib
GRPC_LIB_DIR = $$GRPC_DIR/lib
DEBUG_NAME_POST = ""
CONFIG(debug,debug|release) {
    GRPC_LIB_DIR = $$GRPC_DIR/lib/debug
    DEBUG_NAME_POST = "d"
} else {
}

LIBS += $$GRPC_LIB_DIR/gpr.lib
LIBS += $$GRPC_LIB_DIR/grpc_unsecure.lib
LIBS += $$GRPC_LIB_DIR/grpc++_unsecure.lib
LIBS += $$GRPC_LIB_DIR/libprotobuf$${DEBUG_NAME_POST}.lib
LIBS += $$GRPC_LIB_DIR/zlib.lib
LIBS += Ws2_32.lib
