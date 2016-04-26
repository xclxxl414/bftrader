#include "logger.h"
#include "servicemgr.h"
#include "ui/mainwindow.h"
#include <QApplication>

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
        Logger::startExitMonitor();
        a.setQuitOnLastWindowClosed(false);

        ServiceMgr s;
        s.init();

        MainWindow w;
        w.init();
        w.show();

        result = a.exec();

        w.shutdown();
        s.shutdown();
    }

    return result;
}
