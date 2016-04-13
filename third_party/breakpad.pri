#  Apache 2.0 License
#  copyright 2015 Speedovation, Yash Pal, yash@speedovation.com

INCLUDEPATH += $$PWD $$PWD/breakpad $$PWD/breakpad/src

# Windows
win32{
   HEADERS +=    $$PWD/breakpad/src/common/windows/string_utils-inl.h \
                 $$PWD/breakpad/src/common/windows/guid_string.h \
                 $$PWD/breakpad/src/client/windows/handler/exception_handler.h \
                 $$PWD/breakpad/src/client/windows/common/ipc_protocol.h \
                 $$PWD/breakpad/src/google_breakpad/common/minidump_*.h \
                 $$PWD/breakpad/src/google_breakpad/common/breakpad_types.h \
                 $$PWD/breakpad/src/client/windows/crash_generation/crash_generation_client.h \
                 $$PWD/breakpad/src/common/scoped_ptr.h


    SOURCES +=  $$PWD/breakpad/src/client/windows/handler/exception_handler.cc \
                $$PWD/breakpad/src/common/windows/string_utils.cc \
                $$PWD/breakpad/src/common/windows/guid_string.cc \
                $$PWD/breakpad/src/client/windows/crash_generation/crash_generation_client.cc

}
