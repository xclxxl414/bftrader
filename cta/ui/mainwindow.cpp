#include "mainwindow.h"
#include "dbservice.h"
#include "debug_utils.h"
#include "debugform.h"
#include "finishedorderform.h"
#include "logger.h"
#include "profile.h"
#include "rpcservice.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "tradeform.h"
#include "ui_mainwindow.h"
#include "workingorderform.h"

#include <windows.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(Profile::appName());
    icon_ = QIcon(":/images/cta.png");
    setWindowIcon(icon_);

    //设置trayicon
    this->createActions();
    this->createTrayIcon();

    // actions
    ui->actionNetStart->setEnabled(true);
    ui->actionNetStop->setEnabled(false);

    ui->actionGatewayConnect->setEnabled(true);
    ui->actionGatewayDisconnect->setEnabled(false);

    ui->actionDatafeedConnect->setEnabled(true);
    ui->actionDatafeedDisconnect->setEnabled(false);

    ui->actionCtaStart->setEnabled(true);
    ui->actionCtaStop->setEnabled(false);

    // tabs
    debugForm_ = new DebugForm(this);
    workingOrderForm_ = new WorkingOrderForm(this);
    finishedOrderForm_ = new FinishedOrderForm(this);
    tradeForm_ = new TradeForm(this);

    ui->tabWidgetTrade->addTab(tradeForm_, "trade");
    ui->tabWidgetFinished->addTab(finishedOrderForm_, "finishedOrder");
    ui->tabWidgetWorking->addTab(workingOrderForm_, "workingOrder");
    ui->tabWidgetLog->addTab(debugForm_, "log");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    debugForm_->init();
    finishedOrderForm_->init();
    workingOrderForm_->init();
    tradeForm_->init();
}

void MainWindow::shutdown()
{
    debugForm_->shutdown();
    finishedOrderForm_->shutdown();
    workingOrderForm_->shutdown();
    tradeForm_->shutdown();
}

void MainWindow::on_actionAppVersion_triggered()
{
    BfLog(QString("application's buildtime: ") + QString(__DATE__) + " " + QString(__TIME__));
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

void MainWindow::on_actionNetStart_triggered()
{
    ui->actionNetStart->setEnabled(false);
    ui->actionNetStop->setEnabled(true);
    QMetaObject::invokeMethod(g_sm->rpcService(), "start", Qt::QueuedConnection);
}

void MainWindow::on_actionNetStop_triggered()
{
    ui->actionNetStart->setEnabled(true);
    ui->actionNetStop->setEnabled(false);
    QMetaObject::invokeMethod(g_sm->rpcService(), "stop", Qt::QueuedConnection);
}

void MainWindow::on_actionDatafeedConnect_triggered()
{
    ui->actionDatafeedConnect->setEnabled(false);
    ui->actionDatafeedDisconnect->setEnabled(true);

    QString endpoint = "localhost:50052";
    QString clientId = "cta";
    QMetaObject::invokeMethod(g_sm->dbService(), "connectDatafeed", Qt::QueuedConnection, Q_ARG(QString, endpoint), Q_ARG(QString, clientId));
}

void MainWindow::on_actionDatafeedDisconnect_triggered()
{
    ui->actionDatafeedConnect->setEnabled(true);
    ui->actionDatafeedDisconnect->setEnabled(false);

    QMetaObject::invokeMethod(g_sm->dbService(), "disconnectDatafeed", Qt::QueuedConnection);
}

void MainWindow::on_actionGatewayConnect_triggered()
{
    ui->actionGatewayConnect->setEnabled(false);
    ui->actionGatewayDisconnect->setEnabled(true);

    QString gatewayId = "ctpgateway";
    QString endpoint = "localhost:50051";
    BfConnectPushReq req;
    req.set_clientid("cta");
    req.set_exchange("*");
    req.set_symbol("*");
    req.set_tradehandler(true);
    req.set_tickhandler(true);
    req.set_loghandler(true);
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "connectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId), Q_ARG(QString, endpoint), Q_ARG(BfConnectPushReq, req));
}

void MainWindow::on_actionGatewayDisconnect_triggered()
{
    ui->actionGatewayConnect->setEnabled(true);
    ui->actionGatewayDisconnect->setEnabled(false);

    QString gatewayId = "ctpgateway";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "disconnectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId));
}

// TODO(hege): do it
void MainWindow::on_actionCtaStart_triggered()
{
    ui->actionCtaStart->setEnabled(false);
    ui->actionCtaStop->setEnabled(true);
}

// TODO(hege): do it
void MainWindow::on_actionCtaStop_triggered()
{
    ui->actionCtaStart->setEnabled(true);
    ui->actionCtaStop->setEnabled(false);
}
