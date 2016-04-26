#include "logger.h"
#include "CrashHandler.h"
#include "debug_utils.h"
#include "file_utils.h"
#include "mhook-lib/mhook.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QtGlobal>
#include <crtdbg.h>

static bool g_stopExitMonitor = false;

static void myExit()
{
    if (!g_stopExitMonitor) {
        g_stopExitMonitor = true;
        base::debug::StackTrace stacktrace;
        base::debug::Alias(&stacktrace);
        CrashManager::CrashHandler::instance()->writeMinidump();
    }
}

typedef VOID(__stdcall* fn_ExitProcess)(UINT uExitCode);
typedef BOOL(__stdcall* fn_TerminateProcess)(HANDLE hProcess, UINT uExitCode);
static fn_ExitProcess oldExitProcess = (fn_ExitProcess)::GetProcAddress(::GetModuleHandleW(L"kernel32"), "ExitProcess");
static fn_TerminateProcess oldTerminateProcess = (fn_TerminateProcess)::GetProcAddress(::GetModuleHandleW(L"kernel32"), "TerminateProcess");

static VOID __stdcall myExitProcess(UINT uExitCode)
{
    if (!g_stopExitMonitor) {
        g_stopExitMonitor = true;
        base::debug::StackTrace stacktrace;
        base::debug::Alias(&stacktrace);
        CrashManager::CrashHandler::instance()->writeMinidump();
    }

    oldExitProcess(uExitCode);
}

static BOOL __stdcall myTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    if (::GetProcessId(hProcess) != ::GetCurrentProcessId()) {
        return oldTerminateProcess(hProcess, uExitCode);
    }

    if (!g_stopExitMonitor) {
        g_stopExitMonitor = true;
        base::debug::StackTrace stacktrace;
        base::debug::Alias(&stacktrace);
        CrashManager::CrashHandler::instance()->writeMinidump();
    }

    return oldTerminateProcess(hProcess, uExitCode);
}

void Logger::startExitMonitor()
{
#ifndef _DEBUG
    // Disable the message box for assertions.(see setcrt)
    _CrtSetReportMode(_CRT_ASSERT, 0);

    // Preserve existing error mode, as discussed at http://t/dmea
    UINT new_flags = SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX;
    UINT existing_flags = SetErrorMode(new_flags);
    SetErrorMode(existing_flags | new_flags);
#endif
    /*
#ifdef Q_OS_WIN64
    QString reporter = "C:/Program Files (x86)/Windows Kits/8.0/Debuggers/x64/windbg.exe";
#else
    QString reporter = "C:/Program Files (x86)/Windows Kits/8.0/Debuggers/x86/windbg.exe";
#endif
    QString params = " -z";
    if (!QDir().exists(reporter)) {
        reporter = "c:/windows/system32/notepad.exe";
        params = "";
    }
*/
    QString reporter = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/bugreport.exe"));
    QString params = "";
    CrashManager::CrashHandler::instance()->Init(qApp->applicationDirPath(), reporter, params);

    atexit(myExit);
    Mhook_SetHook((PVOID*)&oldExitProcess, myExitProcess);
    Mhook_SetHook((PVOID*)&oldTerminateProcess, myTerminateProcess);
}

void Logger::stopExitMonitor()
{
    g_stopExitMonitor = true;
    Mhook_Unhook((PVOID*)&oldExitProcess);
    Mhook_Unhook((PVOID*)&oldTerminateProcess);
}

static QtMessageHandler preMessageHandler = nullptr;
static Logger* g_logger = nullptr;

static void myMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
#ifndef _DEBUG
    if (type == QtFatalMsg && g_logger) {
        g_logger->info(msg);
        __debugbreak();
    }
#endif
    preMessageHandler(type, context, msg);
}

Logger::Logger(QObject* parent)
    : QObject(parent)
{
}

static QString Profile_appName()
{
    QFileInfo fi(QCoreApplication::applicationFilePath());
#ifdef _DEBUG
    return fi.baseName() + QStringLiteral("-debug");
#else
    return fi.baseName();
#endif
}

static QString Profile_logPath()
{
    //return QDir::home().absoluteFilePath(Profile_appName() + QStringLiteral("/log.txt"));
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(Profile_appName() + QStringLiteral("/log.txt"));
}

void Logger::init()
{
    QString logFileName = Profile_logPath();
    mkDir(logFileName);
    log_.setFileName(logFileName);
    log_.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append | QIODevice::Unbuffered);
    this->debug(__FUNCTION__);
    g_logger = this;

    preMessageHandler = qInstallMessageHandler(myMessageHandler);
}

void Logger::shutdown()
{
    this->debug(__FUNCTION__);
    qInstallMessageHandler(nullptr);
    g_logger = nullptr;
    log_.close();
}

void Logger::error(QString msg)
{
    QString when = QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss.zzz");
    QString logToFile = when + QStringLiteral("<error>") + msg + QStringLiteral("\n");

    // write to file
    mutex_.lock();
    log_.write(logToFile.toUtf8().constData());
    log_.flush();
    mutex_.unlock();

    // dispatch...
    emit gotError(when, msg);
}

void Logger::info(QString msg)
{
    QString when = QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss.zzz");
    QString logToFile = when + QStringLiteral("<info>") + msg + QStringLiteral("\n");

    // write to file
    mutex_.lock();
    log_.write(logToFile.toUtf8().constData());
    log_.flush();
    mutex_.unlock();

    // dispatch...
    emit gotInfo(when, msg);
}

void Logger::debug(QString msg)
{
    QString when = QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss.zzz");
    QString logToFile = when + QStringLiteral("<debug>") + msg + QStringLiteral("\n");

    // write to file
    mutex_.lock();
    log_.write(logToFile.toUtf8().constData());
    log_.flush();
    mutex_.unlock();

    // dispatch...
    emit gotDebug(when, msg);
}
