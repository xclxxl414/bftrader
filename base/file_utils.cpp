#include "file_utils.h"

#include <QDir>
#include <QFile>

void mkDir(QString local_file)
{
    QFileInfo info(local_file);
    QString dirPath = info.absoluteDir().path();
    QDir dir;
    dir.mkpath(dirPath);
}
