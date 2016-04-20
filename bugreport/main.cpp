#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (!qEnvironmentVariableIsSet("bfcrashreport_apppath") || !qEnvironmentVariableIsSet("bfcrashreport_dumppath") || !qEnvironmentVariableIsSet("bfcrashreport_website")) {
        return 0;
    }
    MainWindow w;
    w.move((QApplication::desktop()->width() - w.width()) / 2, (QApplication::desktop()->height() - w.height()) / 3);
    w.show();

    return a.exec();
}
