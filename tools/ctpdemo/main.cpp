#include "ui/mainwindow.h"
#include "servicemgr.h"
#include <QApplication>
#include "logger.h"

//qt的插件在退出时候没有释放造成泄漏，改了两个太无聊了不改了=
#if 0
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> //_CrtSetBreakAlloc
#endif

int main(int argc, char* argv[])
{
#if 0
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    QApplication a(argc, argv);
    Logger::startExitMonitor();
    a.setQuitOnLastWindowClosed(false);

    ServiceMgr s;
    s.init();

    MainWindow w;
    w.init();
    w.show();

    int result = a.exec();

    w.shutdown();
    s.shutdown();

    return result;
}
