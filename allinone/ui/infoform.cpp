#include "infoform.h"
#include "logger.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_infoform.h"

InfoForm::InfoForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::InfoForm)
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

InfoForm::~InfoForm()
{
    delete ui;
}

void InfoForm::init()
{
    // logger
    QObject::connect(g_sm->logger(), &Logger::gotError, this, &InfoForm::onLog);
    QObject::connect(g_sm->logger(), &Logger::gotInfo, this, &InfoForm::onLog);
}

void InfoForm::shutdown()
{
}

void InfoForm::onLog(QString when, QString msg)
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
