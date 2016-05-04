#include "workingorderform.h"
#include "tablewidget_helper.h"
#include "ui_workingorderform.h"

WorkingOrderForm::WorkingOrderForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::WorkingOrderForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "robotId"
               << "symbol"
               << "exchange"
               << "positioin"
               << "price"
               << "volume";

    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

WorkingOrderForm::~WorkingOrderForm()
{
    delete ui;
}

void WorkingOrderForm::init()
{
}

void WorkingOrderForm::shutdown()
{
}
