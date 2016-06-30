#include "mainwindow.h"
#include "configdialog.h"
#include "dbservice.h"
#include "debug_utils.h"
#include "gatewaymgr.h"
#include "logger.h"
#include "profile.h"
#include "rpcservice.h"
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
    icon_ = QIcon(":/images/gateway.png");
    setWindowIcon(icon_);

    //设置trayicon
    this->createActions();
    this->createTrayIcon();

    // ui actions
    ui->actionBtStart->setEnabled(true);
    ui->actionBtConfig->setEnabled(true);
    ui->actionBtStop->setEnabled(false);

    ui->actionNetStart->setEnabled(true);
    ui->actionNetStop->setEnabled(false);

    ui->actionDfConnect->setEnabled(true);
    ui->actionDfDisconnect->setEnabled(false);

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

void MainWindow::on_actionBtStart_triggered()
{
    //更新ui
    ui->actionBtStart->setEnabled(false);
    ui->actionBtConfig->setEnabled(false);
    ui->actionBtStop->setEnabled(true);

    BfGetTickReq req;
    req.set_symbol(g_sm->profile()->get("symbol").toString().toStdString());
    req.set_exchange(g_sm->profile()->get("exchange").toString().toStdString());
    req.set_fromdate(g_sm->profile()->get("fromDate").toString().toStdString());
    req.set_fromtime(g_sm->profile()->get("fromTime").toString().toStdString());
    req.set_todate(g_sm->profile()->get("toDate").toString().toStdString());
    req.set_totime(g_sm->profile()->get("toTime").toString().toStdString());

    if (req.symbol().length() == 0) {
        BfLog("invalid param,please: backtest-->btConfig first");
        return;
    }

    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "start", Qt::QueuedConnection, Q_ARG(BfGetTickReq, req));
}

void MainWindow::on_actionBtStop_triggered()
{
    //更新ui
    ui->actionBtStart->setEnabled(true);
    ui->actionBtConfig->setEnabled(true);
    ui->actionBtStop->setEnabled(false);

    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "stop", Qt::QueuedConnection);
}

void MainWindow::on_actionBtConfig_triggered()
{
    ConfigDialog dlg(this);
    dlg.load();
    if (dlg.exec()) {
        dlg.save();
    }
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

void MainWindow::on_actionDfConnect_triggered()
{
    ui->actionDfConnect->setEnabled(false);
    ui->actionDfDisconnect->setEnabled(true);

    QString endpoint = "localhost:50052";
    QString clientId = "btgateway";
    QMetaObject::invokeMethod(g_sm->dbService(), "connectDatafeed", Qt::QueuedConnection, Q_ARG(QString, endpoint), Q_ARG(QString, clientId));
}

void MainWindow::on_actionDfDisconnect_triggered()
{
    ui->actionDfConnect->setEnabled(true);
    ui->actionDfDisconnect->setEnabled(false);
    QMetaObject::invokeMethod(g_sm->dbService(), "disconnectDatafeed", Qt::QueuedConnection);
}
