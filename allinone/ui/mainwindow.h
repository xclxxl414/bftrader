#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class Profile;
class InfoForm;
class ErrorForm;
class DebugForm;
class ContractForm;
class FinishedOrderForm;
class WorkingOrderForm;
class PositionForm;
class TradeForm;
class TickForm;
class AccountForm;
class CtaForm;

template <typename T>
class QFutureInterface;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    void init();
    void shutdown();

public slots:
    void onTradeWillBegin();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_actionAppQuit_triggered();
    void on_actionAppVersion_triggered();
    void on_actionPureCallCrash_triggered();
    void on_actionInvalidParamCrash_triggered();
    void on_actionDerefZeroCrash_triggered();
    void on_actionQFatal_triggered();
    void on_actiondebugbreak_triggered();
    void on_actionDebugBreak_triggered();
    void on_actionExit_triggered();
    void on_actionExitProcess_triggered();
    void on_actionTerminateProcess_triggered();
    void on_actionExternal_triggered();
    void on_actionCtpVersion_triggered();
    void on_actionCtpConfig_triggered();
    void on_actionCtpStart_triggered();
    void on_actionCtpStop_triggered();
    void on_actionNetStart_triggered();
    void on_actionNetStop_triggered();
    void on_actionHistoryContract_triggered();
    void on_actionWebsite_triggered();
    void on_actionFeedback_triggered();

private:
    void closeEvent(QCloseEvent* event) override;
    void createTrayIcon();
    void createActions();
    Profile* profile();

    void runOnExternal();
    void runOnExternalEx(QFutureInterface<void>& future);

private:
    Ui::MainWindow* ui;
    InfoForm* infoForm_;
    ErrorForm* errorForm_;
    DebugForm* debugForm_;
    ContractForm* contractForm_;
    FinishedOrderForm* finishedOrderForm_;
    WorkingOrderForm* workingOrderForm_;
    PositionForm* positionForm_;
    TradeForm* tradeForm_;
    TickForm* tickForm_;
    AccountForm* accountForm_;
    CtaForm* ctaForm_;

private:
    QAction* minimizeAction;
    QAction* maximizeAction;
    QAction* restoreAction;
    QAction* quitAction;

    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;
    QIcon icon_;
};

#endif // MAINWINDOW_H
