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
*/

#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QtCore/QString>

namespace CrashManager {
class CrashHandlerPrivate;
class CrashHandler {
public:
    static CrashHandler* instance();
    void Init(const QString& dumpPath, const QString& reporter,const QString& params);

    void setReportCrashesToSystem(bool report);
    bool writeMinidump();

private:
    void setReporter(const QString& reporter,const QString& params);

private:
    CrashHandler();
    ~CrashHandler();
    Q_DISABLE_COPY(CrashHandler)
    CrashHandlerPrivate* d;
};
}

#endif // CRASHHANDLER_H
