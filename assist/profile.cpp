#include "profile.h"
#include "file_utils.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

Profile::Profile(QObject* parent)
    : QObject(parent)
{
}

void Profile::init()
{
    path_ = QDir::home().absoluteFilePath(appName() + QStringLiteral("/config.json"));
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
    return QDir::home().absoluteFilePath(appName() + QStringLiteral("/data/db"));
}

QString Profile::logPath()
{
    return QDir::home().absoluteFilePath(appName() + QStringLiteral("/log.txt"));
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
