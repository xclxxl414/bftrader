/****************************************************************************
**
** Copyright (C) 2007-2015 Speedovation
** Contact: Speedovation Lab (info@speedovation.com)
**
** KineticWing IDE CrashHandler
** http:// kineticwing.com
** This file is part of the core classes of the KiWi Editor (IDE)
**
** Author: Yash Bhardwaj
** License : Apache License 2.0
**
** All rights are reserved.
**
** Work is based on
**   1. https://blog.inventic.eu/2012/08/qt-and-google-breakpad/
**   2. https://github.com/AlekSi/breakpad-qt - Aleksey Palazhchenko, BSD-2 License
**   3. http://www.cplusplus.com/forum/lounge/17684/
**   4. http://www.cplusplus.com/forum/beginner/48283/
**   5. http://www.siafoo.net/article/63
**
*/

#include "CrashHandler.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>

#include <QString>
#include <iostream>

#include "CrashHandler.h"
#include <QString>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>

#include <QDebug>

#if defined(Q_OS_MAC)

#include "client/mac/handler/exception_handler.h"
#include <string.h>

#elif defined(Q_OS_LINUX)

#include "client/linux/handler/exception_handler.h"
#include <sys/types.h>
#include <sys/wait.h>

#elif defined(Q_OS_WIN32)

#include "Tchar.h"
#include "client/windows/handler/exception_handler.h"

#endif

namespace {
// Minidump with stacks, PEB, TEB, and unloaded module list.
const MINIDUMP_TYPE kSmallDumpType = static_cast<MINIDUMP_TYPE>(
    MiniDumpWithProcessThreadData | // Get PEB and TEB.
    MiniDumpWithHandleData | MiniDumpWithUnloadedModules); // Get unloaded modules when available.

// Minidump with all of the above, plus memory referenced from stack.
const MINIDUMP_TYPE kLargerDumpType = static_cast<MINIDUMP_TYPE>(
    MiniDumpWithProcessThreadData | // Get PEB and TEB.
    MiniDumpWithHandleData | MiniDumpWithUnloadedModules | // Get unloaded modules when available.
    MiniDumpWithIndirectlyReferencedMemory); // Get memory referenced by stack.

// Large dump with all process memory.
const MINIDUMP_TYPE kFullDumpType = static_cast<MINIDUMP_TYPE>(
    MiniDumpWithFullMemory | // Full memory from process.
    MiniDumpWithProcessThreadData | // Get PEB and TEB.
    MiniDumpWithHandleData | // Get all handle information.
    MiniDumpWithUnloadedModules); // Get unloaded modules when available.
}

namespace CrashManager {
/************************************************************************/
/* CrashHandlerPrivate                                                  */
/************************************************************************/
class CrashHandlerPrivate {
public:
    CrashHandlerPrivate()
    {
        pHandler = NULL;
    }

    ~CrashHandlerPrivate()
    {
        delete pHandler;
    }

    void InitCrashHandler(const QString& dumpPath);
    static google_breakpad::ExceptionHandler* pHandler;
    static bool bReportCrashesToSystem;

    static char reporter_[1024];
    static char reporterArguments_[8 * 1024];
};

google_breakpad::ExceptionHandler* CrashHandlerPrivate::pHandler = NULL;
bool CrashHandlerPrivate::bReportCrashesToSystem = false;
char CrashHandlerPrivate::reporter_[1024] = { 0 };
char CrashHandlerPrivate::reporterArguments_[8 * 1024] = { 0 };

/************************************************************************/
/* DumpCallback                                                         */
/************************************************************************/

#ifdef Q_OS_WIN

bool launcher(wchar_t* program)
{

    STARTUPINFO si = {};
    si.cb = sizeof si;

    PROCESS_INFORMATION pi = {};

    if (!CreateProcess(NULL, program, 0, FALSE, 0,
            CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW | DETACHED_PROCESS,
            0, 0, &si, &pi)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
    } else {
        if (::PostThreadMessageW(pi.dwThreadId, WM_QUIT, 0, 0)) // Good
            std::cout << "Request to terminate process has been sent!";

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
#else

bool launcher(const char* program, const char* path)
{
    //Cout is not visible in qtcreator output window..may be QtC.. bug or I don't know
    //Visible on terminal output
    std::cout << "CrashReporter: " << program
              << "Dmppath: " << path;

    pid_t pid = fork(); /* Create a child process */

    switch (pid) {
    case -1: /* Error */
        std::cerr << "Uh-Oh! fork() failed.\n";
        exit(1);
    case 0: /* Child process */
    {

        execl(program, program, path, (char*)0); /* Execute the program */
        std::cerr << "Uh-Oh! execl() failed!";
        /* execl doesn't return unless there's an error */
        //qApp->quit();
        exit(1);
    }
    default: /* Parent process */
        return false;
    }

//Q_UNUSED(path);
#endif

    Q_UNUSED(program);
    return false;
}

#if defined(Q_OS_WIN32)
bool DumpCallback(const wchar_t* _dump_dir, const wchar_t* _minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool success)
#elif defined(Q_OS_LINUX)
bool DumpCallback(const google_breakpad::MinidumpDescriptor& md, void* context, bool success)
#elif defined(Q_OS_MAC)
bool DumpCallback(const char* _dump_dir, const char* _minidump_id, void* context, bool success)
#endif
{
    Q_UNUSED(context);
#if defined(Q_OS_WIN32)
    Q_UNUSED(_dump_dir);
    Q_UNUSED(_minidump_id);
    Q_UNUSED(assertion);
    Q_UNUSED(exinfo);
#endif

#if defined(Q_OS_LINUX)
    const char* path = CrashHandlerPrivate::pHandler->minidump_descriptor().path();
    launcher(CrashHandlerPrivate::reporter_, path);
#elif defined(Q_OS_WIN)

    const size_t cSize = strlen(CrashHandlerPrivate::reporter_) + 1;
    wchar_t* program = new wchar_t[cSize];
    mbstowcs(program, CrashHandlerPrivate::reporter_, cSize);
    wchar_t* dpath = new wchar_t[MAX_PATH * 10];
    wcscpy(dpath, _dump_dir);
    wcscat(dpath, L"/");
    wcscat(dpath, _minidump_id);
    wcscat(dpath, L".dmp");
    wchar_t* wpath = new wchar_t[MAX_PATH * 10];
    wcscpy(wpath, program);
    wcscat(wpath, L" ");
    wcscat(wpath, dpath);

    qputenv("bfcrashreport_apppath", QCoreApplication::applicationFilePath().toUtf8());
    qputenv("bfcrashreport_dumppath", QString::fromWCharArray(dpath).toUtf8());
    qputenv("bfcrashreport_website", QString("https://github.com/sunwangme/bftrader/issues").toUtf8());

    launcher(wpath);
    delete[] program;
    delete[] wpath;
    delete[] dpath;
#elif defined(Q_OS_MAC)

    char* path;

    strcpy(path, _dump_dir);
    strcat(path, "/");
    strcat(path, _minidump_id);
    strcat(path, ".dmp");

    launcher(CrashHandlerPrivate::reporter_, path);

#endif

    return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
}

void CrashHandlerPrivate::InitCrashHandler(const QString& dumpPath)
{
    //if already init then skip rest
    if (pHandler != NULL)
        return;

#if defined(Q_OS_WIN32)
    std::wstring pathAsStr = (const wchar_t*)dumpPath.utf16();
    pHandler = new google_breakpad::ExceptionHandler(
        pathAsStr,
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/ 0,
        google_breakpad::ExceptionHandler::HANDLER_ALL,
        kLargerDumpType,
        (HANDLE)NULL, // pipe_handle
        NULL); // crash_generation_client

#ifndef _DEBUG
    pHandler->set_consume_invalid_handle_exceptions(true);
    pHandler->set_handle_debug_exceptions(true);
#endif

#elif defined(Q_OS_LINUX)
    std::string pathAsStr = dumpPath.toStdString();
    google_breakpad::MinidumpDescriptor md(pathAsStr);
    pHandler = new google_breakpad::ExceptionHandler(
        md,
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/ 0,
        true,
        -1);
#elif defined(Q_OS_MAC)
    std::string pathAsStr = dumpPath.toStdString();
    pHandler = new google_breakpad::ExceptionHandler(
        pathAsStr,
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/ 0,
        true,
        NULL);
#endif
}

/************************************************************************/
/* CrashHandler                                                         */
/************************************************************************/
CrashHandler* CrashHandler::instance()
{
    static CrashHandler globalHandler;
    return &globalHandler;
}

CrashHandler::CrashHandler()
{
    d = new CrashHandlerPrivate();
}

CrashHandler::~CrashHandler()
{
    delete d;
}

void CrashHandler::setReportCrashesToSystem(bool report)
{
    d->bReportCrashesToSystem = report;
}

bool CrashHandler::writeMinidump()
{
    bool res = d->pHandler->WriteMinidump();
    if (res) {
        qDebug("BreakpadQt: writeMinidump() successed.");
    } else {
        qWarning("BreakpadQt: writeMinidump() failed.");
    }
    return res;
}

void CrashHandler::Init(const QString& dumpPath, const QString& reporter, const QString& params)
{
    d->InitCrashHandler(dumpPath);

    setReporter(reporter, params);
}

void CrashHandler::setReporter(const QString& reporter, const QString& params)
{
    QString rep = reporter;

    if (!QDir::isAbsolutePath(rep)) {

#if defined(Q_OS_MAC)
        qDebug() << qApp->applicationDirPath();
        // TODO(AlekSi) What to do if we are not inside bundle?
        rep = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/") + rep);
#elif defined(Q_OS_LINUX)
        // MAYBE(AlekSi) Better place for Linux? libexec? or what?
        rep = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/") + rep);
#elif defined(Q_OS_WIN)
        // add .exe for Windows if needed
        if (!QDir().exists(rep)) {
            rep = QDir::cleanPath(qApp->applicationDirPath() + QLatin1String("/") + rep + QLatin1String(".exe"));
        }
#endif

        qDebug("BreakpadQt: setReporter: %s -> %s", qPrintable(reporter), qPrintable(rep));
    }

    Q_ASSERT(QDir::isAbsolutePath(rep));

    //Q_ASSERT(QDir().exists(rep));

    rep += params;

    qstrcpy(d->reporter_, QFile::encodeName(rep).data());
}
}
