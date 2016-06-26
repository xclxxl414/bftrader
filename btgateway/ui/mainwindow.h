#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class Profile;
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
    void onLog(QString when, QString msg);

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

    void on_actionBtStart_triggered();
    void on_actionBtStop_triggered();
    void on_actionBtConfig_triggered();

    void on_actionNetStart_triggered();
    void on_actionNetStop_triggered();

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

    QStringList table_col_;
};

#endif // MAINWINDOW_H
