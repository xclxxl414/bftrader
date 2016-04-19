#include "finishedorderform.h"
#include "nofocusdelegate.h"
#include "ui_finishedorderform.h"

FinishedOrderForm::FinishedOrderForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FinishedOrderForm)
{
    ui->setupUi(this);

    //设置列=
    instruments_col_ << "symbol"
                     << "exchange"
                     << "orderId"

                     << "direction"
                     << "offset"
                     << "price"
                     << "totalVolume"
                     << "tradedVolume"
                     << "status"

                     << "orderTime"
                     << "cancelTime";
    this->ui->tableWidget->setColumnCount(instruments_col_.length());
    for (int i = 0; i < instruments_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(instruments_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

FinishedOrderForm::~FinishedOrderForm()
{
    delete ui;
}

void FinishedOrderForm::init()
{
}

void FinishedOrderForm::shutdown()
{
}
