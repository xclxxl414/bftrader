#include "utils.h"

#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QTextCodec>
#include <windows.h>

void mkDir(QString local_file)
{
    QFileInfo info(local_file);
    QString dirPath = info.absoluteDir().path();
    QDir dir;
    dir.mkpath(dirPath);
}

QString gbk2utf16(char* gbk){
    QTextCodec* codec = QTextCodec::codecForName("gb18030");
    QString utf16 = codec->toUnicode(gbk);
    return utf16;
}

namespace base {
namespace debug {

Base::Base(Derived* derived)
    : derived_(derived) {
}

Base::~Base() {
  derived_->DoSomething();
}

#pragma warning(push)
#pragma warning(disable:4355)
// Disable warning C4355: 'this' : used in base member initializer list.
Derived::Derived()
    : Base(this) {  // C4355
}
#pragma warning(pop)

void Derived::DoSomething() {
}

#if defined(Q_CC_MSVC)
#pragma optimize("", off)
#endif

void Alias(const void* var) {
}

#if defined(Q_CC_MSVC)
#pragma optimize("", on)
#endif

// Disable optimizations for the StackTrace::StackTrace function. It is
// important to disable at least frame pointer optimization ("y"), since
// that breaks CaptureStackBackTrace() and prevents StackTrace from working
// in Release builds (it may still be janky if other frames are using FPO,
// but at least it will make it further).
#if defined(Q_CC_MSVC)
#pragma optimize("", off)
#endif

StackTrace::StackTrace() {
  // When walking our own stack, use CaptureStackBackTrace().
  count_ = CaptureStackBackTrace(0, kMaxTraces, trace_, NULL);
}

#if defined(Q_CC_MSVC)
#pragma optimize("", on)
#endif

}  // namespace debug
}  // namespace base
