#include "encode_utils.h"

#include <QTextCodec>

QString gbk2utf16(const char* gbk)
{
    QTextCodec* codec = QTextCodec::codecForName("gb18030");
    QString utf16 = codec->toUnicode(gbk);
    return utf16;
}
