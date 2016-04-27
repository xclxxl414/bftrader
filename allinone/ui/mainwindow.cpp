#include "mainwindow.h"
#include "accountform.h"
#include "configdialog.h"
#include "contractform.h"
#include "ctpmgr.h"
#include "dbservice.h"
#include "debug_utils.h"
#include "debugform.h"
#include "errorform.h"
#include "finishedorderform.h"
#include "historycontractform.h"
#include "infoform.h"
#include "logger.h"
#include "logindialog.h"
#include "positionform.h"
#include "profile.h"
#include "rpcservice.h"
#include "runextensions.h"
#include "servicemgr.h"
#include "tickform.h"
#include "tradeform.h"
#include "ui_mainwindow.h"
#include "workingorderform.h"
#include "ctaform.h"

#include <QDesktopServices>
#include <QUrl>
#include <QtConcurrentRun>
#include <functional>
#include <windows.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(Profile::appName());
    icon_ = QIcon(":/images/heart.png");
    setWindowIcon(icon_);

    //设置trayicon
    this->createActions();
    this->createTrayIcon();

    // ui actions
    ui->actionCtpStart->setEnabled(true);
    ui->actionCtpConfig->setEnabled(true);
    ui->actionCtpStop->setEnabled(false);

    ui->actionNetStart->setEnabled(true);
    ui->actionNetStop->setEnabled(false);

    // tabs
    infoForm_ = new InfoForm(this);
    errorForm_ = new ErrorForm(this);
    debugForm_ = new DebugForm(this);
    contractForm_ = new ContractForm(this);
    positionForm_ = new PositionForm(this);
    workingOrderForm_ = new WorkingOrderForm(this);
    finishedOrderForm_ = new FinishedOrderForm(this);
    tradeForm_ = new TradeForm(this);
    tickForm_ = new TickForm(this);
    ctaForm_ = new CtaForm(this);

    ui->tabWidgetMarket->addTab(tickForm_, "tick");
    ui->tabWidgetMarket->addTab(contractForm_, "contract");
    ui->tabWidgetMarket->addTab(ctaForm_, "cta");
    ui->tabWidgetLog->addTab(infoForm_, "info");
    ui->tabWidgetLog->addTab(errorForm_, "error");
    ui->tabWidgetLog->addTab(debugForm_, "debug");
    ui->tabWidgetPosition->addTab(positionForm_, "position");
    ui->tabWidgetOrder->addTab(workingOrderForm_, "workingOrder");
    ui->tabWidgetOrder->addTab(finishedOrderForm_, "finishedOrder");
    ui->tabWidgetOrder->addTab(tradeForm_, "trade");

    // statusbar,隐藏那个竖线=
    accountForm_ = new AccountForm(this);
    ui->statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
    ui->statusBar->setSizeGripEnabled(false);
    ui->statusBar->addWidget(accountForm_, 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    // sub window
    infoForm_->init();
    errorForm_->init();
    debugForm_->init();
    ctaForm_->init();
    contractForm_->init();
    tickForm_->init();
    positionForm_->init();
    workingOrderForm_->init();
    finishedOrderForm_->init();
    tradeForm_->init();
    accountForm_->init();

    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &MainWindow::onTradeWillBegin);
}

void MainWindow::shutdown()
{
    // sub window
    infoForm_->shutdown();
    errorForm_->shutdown();
    debugForm_->shutdown();
    ctaForm_->shutdown();
    contractForm_->shutdown();
    tickForm_->shutdown();
    positionForm_->shutdown();
    workingOrderForm_->shutdown();
    finishedOrderForm_->shutdown();
    tradeForm_->shutdown();
    accountForm_->shutdown();
}

void MainWindow::onTradeWillBegin()
{
    BfDebug(__FUNCTION__);
}

void MainWindow::on_actionAppVersion_triggered()
{
    BfInfo(QString("application's buildtime<error>: ") + QString(__DATE__) + " " + QString(__TIME__));
    BfInfo(QString("application's buildtime<info>: ") + QString(__DATE__) + " " + QString(__TIME__));
    BfInfo(QString("application's buildtime<debug>: ") + QString(__DATE__) + " " + QString(__TIME__));
}

void MainWindow::on_actionAppQuit_triggered()
{
    if (g_sm->ctpMgr()->running()) {
        this->showNormal();
        BfInfo("please stop ctp first");
        return;
    }

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
    connect(quitAction, SIGNAL(triggered()), this, SLOT(on_actionQuit_triggered()));
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

void MainWindow::on_actionInvalidParamCrash_triggered()
{
    //InvalidParamCrash
    printf(nullptr);
}

void MainWindow::on_actionPureCallCrash_triggered()
{
    //PureCallCrash
    base::debug::Derived derived;
    base::debug::Alias(&derived);
}

void MainWindow::on_actionDerefZeroCrash_triggered()
{
    //DerefZeroCrash
    int* x = 0;
    *x = 1;
    base::debug::Alias(x);
}

void MainWindow::on_actionQFatal_triggered()
{
    qFatal("crash for qFatal");
}

void MainWindow::on_actiondebugbreak_triggered()
{
    __debugbreak();
}

void MainWindow::on_actionDebugBreak_triggered()
{
    DebugBreak();
}

void MainWindow::on_actionExit_triggered()
{
    exit(1);
}

void MainWindow::on_actionExitProcess_triggered()
{
    ::ExitProcess(1);
}

void MainWindow::on_actionTerminateProcess_triggered()
{
    ::TerminateProcess(::GetCurrentProcess(), 1);
}

void MainWindow::on_actionExternal_triggered()
{
    QFuture<void> future1 = QtConcurrent::run(this, &MainWindow::runOnExternal);
    QFuture<void> future2 = QtConcurrent::run(std::bind(&MainWindow::runOnExternal, this));
    QFuture<void> future3 = QtConcurrent::run(&MainWindow::runOnExternalEx, this);
    std::function<void(QFutureInterface<void>&)> fn = std::bind(&MainWindow::runOnExternalEx, this, std::placeholders::_1);
    QFuture<void> future4 = QtConcurrent::run(fn);

    Q_UNUSED(future1);
    Q_UNUSED(future2);
    Q_UNUSED(future3);
    Q_UNUSED(future4);
}

void MainWindow::runOnExternal()
{
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    BfInfo(__FUNCTION__);
}

void MainWindow::runOnExternalEx(QFutureInterface<void>& future)
{
    g_sm->checkCurrentOn(ServiceMgr::EXTERNAL);
    BfInfo(__FUNCTION__);

    future.reportFinished();
}

void MainWindow::on_actionCtpVersion_triggered()
{
    QMetaObject::invokeMethod(g_sm->ctpMgr(), "showVersion", Qt::QueuedConnection);
}

void MainWindow::on_actionCtpConfig_triggered()
{
    ConfigDialog dlg(this);
    dlg.load();
    if (dlg.exec()) {
        dlg.save();
    }
}

void MainWindow::on_actionCtpStart_triggered()
{
    // input password
    LoginDialog dlg;
    if (!dlg.exec()) {
        return;
    }
    QString password = dlg.getPassword();

    //更新ui,接收数据中不要出现模态对话框=
    ui->actionCtpStart->setEnabled(false);
    ui->actionCtpConfig->setEnabled(false);
    ui->actionCtpStop->setEnabled(true);

    QMetaObject::invokeMethod(g_sm->ctpMgr(), "start", Qt::QueuedConnection, Q_ARG(QString, password));
}

void MainWindow::on_actionCtpStop_triggered()
{
    //更新ui
    ui->actionCtpStart->setEnabled(true);
    ui->actionCtpConfig->setEnabled(true);
    ui->actionCtpStop->setEnabled(false);

    QMetaObject::invokeMethod(g_sm->ctpMgr(), "stop", Qt::QueuedConnection);
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

void MainWindow::on_actionHistoryContract_triggered()
{
    HistoryContractForm* form = new HistoryContractForm();
    form->setWindowFlags(Qt::Window);
    form->init();
    form->show();
}

void MainWindow::on_actionWebsite_triggered()
{
    QUrl url("https://github.com/sunwangme/bftrader");
    QDesktopServices::openUrl(url);
}

void MainWindow::on_actionFeedback_triggered()
{
    QUrl url("https://github.com/sunwangme/bftrader/issues");
    QDesktopServices::openUrl(url);
}
