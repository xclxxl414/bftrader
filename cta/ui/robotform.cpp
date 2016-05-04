#include "robotform.h"
#include "tablewidget_helper.h"
#include "ui_robotform.h"

RobotForm::RobotForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RobotForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "robotId"
               << "status"
               << "modelId"
               << "symbol"
               << "exchange"
               << "gatewayId"
               << "maxPos"
               << "maxVol"
               << "maxLoss";

    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

RobotForm::~RobotForm()
{
    delete ui;
}

void RobotForm::init()
{
}

void RobotForm::shutdown()
{
}
