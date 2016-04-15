#include "logform.h"
#include "ui_logform.h"
#include "servicemgr.h"
#include "logger.h"

LogForm::LogForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogForm)
{
    ui->setupUi(this);
}

LogForm::~LogForm()
{
    delete ui;
}

void LogForm::init(){
    // logger
    QObject::connect(g_sm->logger(), &Logger::gotInfo, this, &LogForm::onInfo);
}

void LogForm::shutdown(){

}

void LogForm::onInfo(QString when,QString msg)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    QTableWidgetItem* item = nullptr;

    item = new QTableWidgetItem(when);
    ui->tableWidget->setItem(row, 0, item);

    item = new QTableWidgetItem(msg);
    ui->tableWidget->setItem(row, 1, item);
}
