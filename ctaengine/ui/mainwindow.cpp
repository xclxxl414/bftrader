#include "mainwindow.h"
#include "ctpmgr.h"
#include "dbservice.h"
#include "debug_utils.h"
#include "logger.h"
#include "profile.h"
#include "runextensions.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_mainwindow.h"
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

void MainWindow::on_actionVersion_triggered()
{
    BfError(QString("application's buildtime<error>: ") + QString(__DATE__) + " " + QString(__TIME__));
    BfInfo(QString("application's buildtime<info>: ") + QString(__DATE__) + " " + QString(__TIME__));
    BfDebug(QString("application's buildtime<debug>: ") + QString(__DATE__) + " " + QString(__TIME__));
}

void MainWindow::on_actionQuit_triggered()
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

void MainWindow::on_actionDbOpen_triggered()
{
    QMetaObject::invokeMethod(g_sm->dbService(), "dbOpen", Qt::QueuedConnection);
}

void MainWindow::on_actionDbInit_triggered()
{
    QMetaObject::invokeMethod(g_sm->dbService(), "dbInit", Qt::QueuedConnection);
}

void MainWindow::on_actionDbClose_triggered()
{
    QMetaObject::invokeMethod(g_sm->dbService(), "dbClose", Qt::QueuedConnection);
}
