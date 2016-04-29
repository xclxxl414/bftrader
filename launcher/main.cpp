#include "encode_utils.h"
#include "logger.h"
#include "profile.h"
#include "servicemgr.h"
#include "ui/mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>

//开一个vc编译器cmd，执行：windeployqt --dir bftrader --no-angle --no-translations --pdb appname.exe

//qt的插件在退出时候没有释放造成泄漏，改了两个太无聊了不改了=
#if 0
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h> //_CrtSetBreakAlloc
#include <stdlib.h>
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#if 0
#include "utils.h"
#include <QMap>
#include <QMutex>
#include <private/qhooks_p.h>

static QMap<QObject*,base::debug::StackTrace> s_obs;
static QMutex s_obs_mutex;

static void myExit(){
    auto obs = &s_obs;
    if (obs->count()>=2){
        //breakpad已经被析构了还是用windbg直接挂了当=
        //CrashManager::CrashHandler::instance()->writeMinidump();
        //dps st0
        //dps st1
        //dv /v
        //dt /v st0
        base::debug::StackTrace st0 = obs->values().at(0);
        base::debug::StackTrace st1 = obs->values().at(1);
        base::debug::Alias(&st0);
        base::debug::Alias(&st1);
        __debugbreak();
    }
}

void myAddQObjectCallback(QObject* o){
    auto obs = &s_obs;
    base::debug::StackTrace st;
    s_obs_mutex.lock();
    obs->insert(o,st);
    s_obs_mutex.unlock();
}

void myRemoveQObjectCallback(QObject* o){
    auto obs = &s_obs;
    s_obs_mutex.lock();
    obs->remove(o);
    s_obs_mutex.unlock();
}

void myStartupCallback(){
    int i = 0;
    base::debug::Alias(&i);
}
#endif

int main(int argc, char* argv[])
{
#if 0
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#if 0
    qtHookData[QHooks::AddQObject] = (quintptr)&myAddQObjectCallback;
    qtHookData[QHooks::RemoveQObject] = (quintptr)&myRemoveQObjectCallback;
    qtHookData[QHooks::Startup] = (quintptr)&myStartupCallback;
    atexit(myExit);
#endif

    int result = 0;
    {
        QApplication a(argc, argv);

        // ctp dont support path include cjk
        if (hasCJK(a.applicationDirPath())) {
            QMessageBox::critical(nullptr, Profile::appName(), "dont support cjk dir");
            return -1;
        }

        // single instance for dir+appname
        if (!Profile::checkSingleInstance()) {
            QMessageBox::critical(nullptr, Profile::appName(), "dont support 1+ instance");
            return -1;
        }

        // crash monitor
        Logger::startExitMonitor();

        // windows
        a.setQuitOnLastWindowClosed(false);

        // servicemgr
        ServiceMgr s;
        s.init();

        // main window
        MainWindow w;
        w.init();
        w.move((QApplication::desktop()->width() - w.width()) / 2, (QApplication::desktop()->height() - w.height()) / 3);
        w.show();

        // message pump
        result = a.exec();

        // shutdown
        w.shutdown();
        s.shutdown();

        // free single instance mutex
        Profile::closeSingleInstanceMutex();
    }

    return result;
}
