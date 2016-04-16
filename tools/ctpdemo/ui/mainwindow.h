#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QVariantMap>

namespace Ui {
class MainWindow;
}
class Profile;
class Logger;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    //构造和析构做ui,init和shutdown做逻辑，析构函数时逻辑已经全部shutdown=
    //ui的shutdown早于逻辑的shutdown=
    //不要保留service的指针，最好每次都调用函数来取，便于做检查=
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    void init();
    void shutdown();

signals:
    void startCtp(QString password);
    void stopCtp();

public slots:
    void onInfo(QString when, QString msg);
    void onGotInstruments(QStringList ids);
    void onGotTick(void* tick, int indexRb, void* rb);
    void resetUI();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_actionVersion_triggered();
    void on_actionConfig_triggered();
    void on_actionStart_triggered();
    void on_actionStop_triggered();
    void on_actionQuit_triggered();
    void on_actionInvalidParamCrash_triggered();
    void on_actionPureCallCrash_triggered();
    void on_actionDerefZeroCrash_triggered();
    void on_actionQFatal_triggered();
    void on_actionDebugbreak_triggered();
    void on_actionDebugBreak_triggered();
    void on_actionExit_triggered();
    void on_actionExitProcess_triggered();
    void on_actionTerminateProcess_triggered();

    void on_tableWidget_cellDoubleClicked(int row, int column);
    void on_actionInstruments_triggered();

private:
    void closeEvent(QCloseEvent* event) override;
    void createTrayIcon();
    void createActions();
    Profile* profile();
    Logger* logger();

private:
    Ui::MainWindow* ui;
    QMap<QString, int> instruments_row_;
    QStringList instruments_col_;

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
