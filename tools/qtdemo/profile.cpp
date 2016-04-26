#include "profile.h"
#include "file_utils.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <windows.h>

Profile::Profile(QObject* parent)
    : QObject(parent)
{
}

void Profile::init()
{
    //path_ = QDir::home().absoluteFilePath(appName() + QStringLiteral("/config.json"));
    path_ = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(appName() + QStringLiteral("/config.json"));
    mkDir(path_);

    QFile file(path_);
    bool res = file.open(QIODevice::ReadOnly);
    if (!res) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    store_ = obj.toVariantMap();
}

void Profile::shutdown()
{
    commit();
}

QVariant Profile::get(QString k, QVariant defaultValue)
{
    return store_.value(k, defaultValue);
}

void Profile::put(QString k, QVariant v)
{
    QVariant old = get(k);
    store_.insert(k, v);
    if (old != v) {
        emit keyValueChanged(k, v);
    }
    dirty_ = true;
}

void Profile::commit()
{
    if (!dirty_) {
        return;
    }

    QJsonObject obj = QJsonObject::fromVariantMap(store_);
    QJsonDocument doc = QJsonDocument(obj);
    QFile file(path_);
    int res = file.open(QIODevice::WriteOnly);
    if (!res) {
        return;
    }
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
    dirty_ = false;
}

QString Profile::dbPath()
{
    //return QDir::home().absoluteFilePath(appName() + QStringLiteral("/data/db"));
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(appName() + QStringLiteral("/data/db"));
}

QString Profile::logPath()
{
    //return QDir::home().absoluteFilePath(appName() + QStringLiteral("/log.txt"));
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(appName() + QStringLiteral("/log.txt"));
}

QString Profile::appName()
{
    QFileInfo fi(QCoreApplication::applicationFilePath());
#ifdef _DEBUG
    return fi.baseName() + QStringLiteral("-debug");
#else
    return fi.baseName();
#endif
}

static HANDLE hSingleInstanceMutex = nullptr;
bool Profile::checkSingleInstance()
{
    QString logPath = QString("Global//") + Profile::logPath();
    HANDLE hMutex = CreateMutexW(NULL, FALSE, logPath.toStdWString().c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return false;
    }
    hSingleInstanceMutex = hMutex;
    return true;
}

void Profile::closeSingleInstanceMutex()
{
    if (hSingleInstanceMutex) {
        CloseHandle(hSingleInstanceMutex);
        hSingleInstanceMutex = nullptr;
    }
}
