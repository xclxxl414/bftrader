#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    account_label_ = new QLabel();
    account_label_->setText("账号：______");

    cash_label_ = new QLabel();
    cash_label_->setText("资金：______");

    position_label_ = new QLabel();
    position_label_->setText("持仓：______");

    percent_label_ = new QLabel();
    percent_label_->setText("仓位：______");

    ui->statusBar->addWidget(percent_label_);
    ui->statusBar->insertWidget(0,position_label_);
    ui->statusBar->insertWidget(0,cash_label_);
    ui->statusBar->insertWidget(0,account_label_);

    //ui->statusBar->showMessage("helloworld");

    ui->statusBar->setSizeGripEnabled(false);
    ui->statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}
