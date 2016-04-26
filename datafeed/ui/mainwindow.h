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
    void on_actionQuit_triggered();
    void on_actionVersion_triggered();
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
    void on_actionDbOpen_triggered();
    void on_actionDbInit_triggered();
    void on_actionDbClose_triggered();

private:
    void closeEvent(QCloseEvent* event) override;
    void createTrayIcon();
    void createActions();
    Profile* profile();

    void runOnExternal();
    void runOnExternalEx(QFutureInterface<void>& future);

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
