#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString appPath = qgetenv("bfcrashreport_apppath");
    QString dumpPath = qgetenv("bfcrashreport_dumppath");
    QString webSite = qgetenv("bfcrashreport_website");

    qunsetenv("bfcrashreport_apppath");
    qunsetenv("bfcrashreport_dumppath");
    qunsetenv("bfcrashreport_website");

    QString appName = QFileInfo(appPath).baseName();
    this->setWindowTitle(appName + " crashed!!!");
    ui->lineEditAppPath->setText(appPath);
    ui->lineEditDumpPath->setText(dumpPath);
    ui->lineEditWebSite->setText(webSite);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonClose_clicked()
{
    this->close();
}

void MainWindow::on_pushButtonFeedback_clicked()
{
    QUrl url(ui->lineEditWebSite->text());
    QDesktopServices::openUrl(url);
}
