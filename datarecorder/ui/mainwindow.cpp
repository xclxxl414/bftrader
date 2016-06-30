#include "mainwindow.h"
#include "debug_utils.h"
#include "gatewaymgr.h"
#include "logger.h"
#include "profile.h"
#include "pushservice.h"
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

    // ui actions
    ui->actionGatewayConnect->setEnabled(true);
    ui->actionGatewayDisconnect->setEnabled(false);

    ui->actionDatafeedConnect->setEnabled(true);
    ui->actionDatafeedDisconnect->setEnabled(false);

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
    QObject::connect(g_sm->logger(), &Logger::gotLog, this, &MainWindow::onLog);
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
    BfLog(QString("application's buildtime<debug>: ") + QString(__DATE__) + " " + QString(__TIME__));
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

void MainWindow::on_actionGatewayConnect_triggered()
{
    ui->actionGatewayConnect->setEnabled(false);
    ui->actionGatewayDisconnect->setEnabled(true);

    QString gatewayId = "ctpgateway";
    QString endpoint = "localhost:50051";
    BfConnectPushReq req;
    req.set_clientid("datarecorder");
    req.set_exchange("*");
    req.set_symbol("*");
    req.set_tradehandler(false);
    req.set_tickhandler(true);
    req.set_loghandler(false);
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "connectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId), Q_ARG(QString, endpoint), Q_ARG(BfConnectPushReq, req));
}

void MainWindow::on_actionGatewayDisconnect_triggered()
{
    ui->actionGatewayConnect->setEnabled(true);
    ui->actionGatewayDisconnect->setEnabled(false);

    QString gatewayId = "ctpgateway";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "disconnectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId));
}

void MainWindow::on_actionDatafeedConnect_triggered()
{
    ui->actionDatafeedConnect->setEnabled(false);
    ui->actionDatafeedDisconnect->setEnabled(true);

    QString endpoint = "localhost:50052";
    QString clientId = "datarecorder";
    QMetaObject::invokeMethod(g_sm->pushService(), "connectDatafeed", Qt::QueuedConnection, Q_ARG(QString, endpoint), Q_ARG(QString, clientId));
}

void MainWindow::on_actionDatafeedDisconnect_triggered()
{
    ui->actionDatafeedConnect->setEnabled(true);
    ui->actionDatafeedDisconnect->setEnabled(false);

    QMetaObject::invokeMethod(g_sm->pushService(), "disconnectDatafeed", Qt::QueuedConnection);
}
