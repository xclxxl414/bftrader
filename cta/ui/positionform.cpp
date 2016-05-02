#include "positionform.h"
#include "ui_positionform.h"
#include "tablewidget_helper.h"

PositionForm::PositionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PositionForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "robotId"
               << "symbol"
               << "exchange"
               << "positioin"
               << "price"
               << "profit";

    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

PositionForm::~PositionForm()
{
    delete ui;
}

void PositionForm::init()
{

}

void PositionForm::shutdown()
{

}
