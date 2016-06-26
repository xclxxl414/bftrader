#include "mainwindow.h"
#include "debug_utils.h"
#include "gatewaymgr.h"
#include "logger.h"
#include "profile.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_mainwindow.h"
#include <windows.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(Profile::appName());
    icon_ = QIcon(":/images/datafeed.png");
    setWindowIcon(icon_);

    //设置trayicon
    this->createActions();
    this->createTrayIcon();

    //设置列=
    table_col_ << "when"
               << "message";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    // logger
    QObject::connect(g_sm->logger(), &Logger::gotError, this, &MainWindow::onLog);
    QObject::connect(g_sm->logger(), &Logger::gotInfo, this, &MainWindow::onLog);
    QObject::connect(g_sm->logger(), &Logger::gotDebug, this, &MainWindow::onLog);
}

void MainWindow::shutdown()
{
}

void MainWindow::onLog(QString when, QString msg)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    QTableWidgetItem* item = nullptr;

    item = new QTableWidgetItem(when);
    ui->tableWidget->setItem(row, 0, item);

    item = new QTableWidgetItem(msg);
    ui->tableWidget->setItem(row, 1, item);

    ui->tableWidget->scrollToBottom();
}

void MainWindow::on_actionAppVersion_triggered()
{
    BfError(QString("application's buildtime<error>: ") + QString(__DATE__) + " " + QString(__TIME__));
    BfInfo(QString("application's buildtime<info>: ") + QString(__DATE__) + " " + QString(__TIME__));
    BfDebug(QString("application's buildtime<debug>: ") + QString(__DATE__) + " " + QString(__TIME__));
}

void MainWindow::on_actionAppQuit_triggered()
{
    Logger::stopExitMonitor();
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    this->hide();
    event->ignore();
}

void MainWindow::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(on_actionAppQuit_triggered()));
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(icon_);
    trayIcon->setToolTip(Profile::appName());

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon->setVisible(true);
    trayIcon->show();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if (!this->isVisible())
            this->showNormal();
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    default:;
    }
}

Profile* MainWindow::profile()
{
    return g_sm->profile();
}

void MainWindow::on_actionCrashInvalidParamCrash_triggered()
{
    //InvalidParamCrash
    printf(nullptr);
}

void MainWindow::on_actionCrashPureCallCrash_triggered()
{
    //PureCallCrash
    base::debug::Derived derived;
    base::debug::Alias(&derived);
}

void MainWindow::on_actionCrashDerefZeroCrash_triggered()
{
    //DerefZeroCrash
    int* x = 0;
    *x = 1;
    base::debug::Alias(x);
}

void MainWindow::on_actionCrashQFatal_triggered()
{
    qFatal("crash for qFatal");
}

void MainWindow::on_actionCrashdebugbreak_triggered()
{
    __debugbreak();
}

void MainWindow::on_actionCrashDebugBreak_triggered()
{
    DebugBreak();
}

void MainWindow::on_actionCrashExit_triggered()
{
    exit(1);
}

void MainWindow::on_actionCrashExitProcess_triggered()
{
    ::ExitProcess(1);
}

void MainWindow::on_actionCrashTerminateProcess_triggered()
{
    ::TerminateProcess(::GetCurrentProcess(), 1);
}

void MainWindow::on_actionNezipAskData_triggered()
{
    AskDataTag tag;

    // iC 1607 & index
    tag.nezipCode = "ZJIC07";
    tag.ctpSymbol = "IC1607";
    tag.ctpExchange = "CFFEX";
    tag.index = false;
    tag.ctpCloner = "IC1607";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    tag.nezipCode = "ZJIC";
    tag.ctpSymbol = "IC8888";
    tag.ctpExchange = "CFFEX";
    tag.index = true;
    tag.ctpCloner = "IC1607";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    // sr 609 & index
    tag.nezipCode = "ZZSRS09";
    tag.ctpSymbol = "SR609";
    tag.ctpExchange = "CZCE";
    tag.index = false;
    tag.ctpCloner = "SR609";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    tag.nezipCode = "ZZSRS";
    tag.ctpSymbol = "SR888";
    tag.ctpExchange = "CZCE";
    tag.index = true;
    tag.ctpCloner = "SR609";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    // rb 1610 & index
    tag.nezipCode = "SQrb10";
    tag.ctpSymbol = "rb1610";
    tag.ctpExchange = "SHFE";
    tag.index = false;
    tag.ctpCloner = "rb1610";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    tag.nezipCode = "SQrb";
    tag.ctpSymbol = "rb8888";
    tag.ctpExchange = "SHFE";
    tag.index = true;
    tag.ctpCloner = "rb1610";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    // pp 1609 & index
    tag.nezipCode = "DLpp09";
    tag.ctpSymbol = "pp1609";
    tag.ctpExchange = "DCE";
    tag.index = false;
    tag.ctpCloner = "pp1609";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    tag.nezipCode = "DLpp";
    tag.ctpSymbol = "pp8888";
    tag.ctpExchange = "DCE";
    tag.index = true;
    tag.ctpCloner = "pp1609";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    // m 1609 & index
    tag.nezipCode = "DLm09";
    tag.ctpSymbol = "m1609";
    tag.ctpExchange = "DCE";
    tag.index = false;
    tag.ctpCloner = "m1609";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));

    tag.nezipCode = "DLm";
    tag.ctpSymbol = "m8888";
    tag.ctpExchange = "DCE";
    tag.index = true;
    tag.ctpCloner = "m1609";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "askData", Qt::QueuedConnection, Q_ARG(AskDataTag, tag));
}

void MainWindow::on_actionNezipLoadDrv_triggered()
{
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "loadDrv", Qt::QueuedConnection);
}
