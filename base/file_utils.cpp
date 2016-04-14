#include "file_utils.h"

#include <QFile>
#include <QDir>

void mkDir(QString local_file)
{
    QFileInfo info(local_file);
    QString dirPath = info.absoluteDir().path();
    QDir dir;
    dir.mkpath(dirPath);
}
