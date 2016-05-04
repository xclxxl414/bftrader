#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class Profile;
class RobotForm;
class PositionForm;
class InfoForm;
class DebugForm;
class ErrorForm;
class WorkingOrderForm;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    void init();
    void shutdown();

public slots:

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_actionAppQuit_triggered();
    void on_actionAppVersion_triggered();
    void on_actionCrashPureCallCrash_triggered();
    void on_actionCrashInvalidParamCrash_triggered();
    void on_actionCrashDerefZeroCrash_triggered();
    void on_actionCrashQFatal_triggered();
    void on_actionCrashdebugbreak_triggered();
    void on_actionCrashDebugBreak_triggered();
    void on_actionCrashExit_triggered();
    void on_actionCrashExitProcess_triggered();
    void on_actionCrashTerminateProcess_triggered();
    void on_actionCtpConnect_triggered();
    void on_actionCtpDisconnect_triggered();
    void on_actionNetStart_triggered();
    void on_actionNetStop_triggered();
    void on_actionStopAutoTrading_triggered();
    void on_actionStartAutoTrading_triggered();
    void on_actionShowRobotOrder_triggered();
    void on_actionShowRobotTrade_triggered();
    void on_actionAddRobot_triggered();
    void on_actionDeleteRobot_triggered();

private:
    void closeEvent(QCloseEvent* event) override;
    void createTrayIcon();
    void createActions();
    Profile* profile();

private:
    Ui::MainWindow* ui;

private:
    QAction* minimizeAction;
    QAction* maximizeAction;
    QAction* restoreAction;
    QAction* quitAction;

    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;
    QIcon icon_;

    //tabs
    RobotForm* robotForm_;
    PositionForm* positionForm_;
    InfoForm* infoForm_;
    DebugForm* debugForm_;
    ErrorForm* errorForm_;
    WorkingOrderForm* workingOrderForm_;
};

#endif // MAINWINDOW_H
