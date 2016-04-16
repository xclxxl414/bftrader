#include "logform.h"
#include "ui_logform.h"
#include "servicemgr.h"
#include "logger.h"
#include "nofocusdelegate.h"

LogForm::LogForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogForm)
{
    ui->setupUi(this);

    //设置列=
    instruments_col_  << "when"
                      << "message";
    this->ui->tableWidget->setColumnCount(instruments_col_.length());
    for (int i = 0; i < instruments_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(instruments_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true); //最后一览自适应宽度=
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
