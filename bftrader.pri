# warn
win32-msvc*:QMAKE_CXXFLAGS_WARN_ON -= -w34100
win32-msvc*:QMAKE_CXXFLAGS += -wd4819 -wd4267 -wd4244
win32-msvc*:QMAKE_CFLAGS_WARN_ON -= -w34100
win32-msvc*:QMAKE_CFLAGS += -wd4819 -wd4267 -wd4244

# pdb
#win32-msvc*:QMAKE_CFLAGS_RELEASE += -Zi
#win32-msvc*:QMAKE_CXXFLAGS_RELEASE += -Zi
#win32-msvc*:QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF

# pdb
CONFIG += force_debug_info

# vs2015 pdb fastlink
QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO = /DEBUG:FastLink /OPT:REF #/INCREMENTAL:NO

# vs2015 ltcg incremental
CONFIG += ltcg
QMAKE_LFLAGS_LTCG = /LTCG:incremental #/LTCG:incremental_rebuild

# shared
# QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -= /MT
# QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= /MT
# QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += /MD
# QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += /MD
# QMAKE_CFLAGS_DEBUG += /MDd
# QMAKE_CXXFLAGS_DEBUG += /MDd

# posttheadmessage
win32:LIBS += user32.lib

# defines win7+
DEFINES += _WIN32_WINNT=0x0600

# fuck the same name for cpps
CONFIG += no_batch

# c++11
CONFIG += c++11

# x64
contains(QT_ARCH, i386) {
    #message("32-bit")
    DEFINES += WIN32
    DEFINES += _WIN32
    DEFINES += __WIN32__
    error("dont support x86")
} else {
    #message("64-bit")
    DEFINES += WIN64
    DEFINES += _WIN64
    DEFINES += __WIN64__
}
