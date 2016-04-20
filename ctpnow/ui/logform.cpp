#include "logform.h"
#include "logger.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_logform.h"

LogForm::LogForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LogForm)
{
    ui->setupUi(this);

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

LogForm::~LogForm()
{
    delete ui;
}

void LogForm::init()
{
    // logger
    QObject::connect(g_sm->logger(), &Logger::gotInfo, this, &LogForm::onInfo);
}

void LogForm::shutdown()
{
}

void LogForm::onInfo(QString when, QString msg)
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
