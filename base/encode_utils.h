#pragma once

#include <QString>

QString gbk2utf16(const char* gbk);

// 判断unicode字符串是否含有cjk
bool hasCJK(QString str);
