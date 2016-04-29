#include "encode_utils.h"

#include <QTextCodec>

QString gbk2utf16(const char* gbk)
{
    QTextCodec* codec = QTextCodec::codecForName("gb18030");
    QString utf16 = codec->toUnicode(gbk);
    return utf16;
}

// assistant\3rdparty\clucene\src\CLucene\analysis\standard\StandardTokenizer.cpp
/*
#define _CJK			(  (ch>=0x3040 && ch<=0x318f) || \
                         (ch>=0x3300 && ch<=0x337f) || \
                         (ch>=0x3400 && ch<=0x3d2d) || \
                         (ch>=0x4e00 && ch<=0x9fff) || \
                         (ch>=0xf900 && ch<=0xfaff) || \
                         (ch>=0xac00 && ch<=0xd7af) ) //korean
*/

#define _CJK ((ch >= 0x3040 && ch <= 0x318f) || (ch >= 0x3300 && ch <= 0x337f) || (ch >= 0x3400 && ch <= 0x3d2d) || (ch >= 0x4e00 && ch <= 0x9fff) || (ch >= 0xf900 && ch <= 0xfaff) || (ch >= 0xac00 && ch <= 0xd7af))

bool hasCJK(QString str)
{
    for (QChar ch : str) {
        if (_CJK) {
            return true;
        }
    }

    return false;
}
