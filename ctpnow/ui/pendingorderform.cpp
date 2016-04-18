#include "pendingorderform.h"
#include "ui_pendingorderform.h"
#include "nofocusdelegate.h"

PendingOrderForm::PendingOrderForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PendingOrderForm)
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

PendingOrderForm::~PendingOrderForm()
{
    delete ui;
}

void PendingOrderForm::init()
{
}

void PendingOrderForm::shutdown()
{
}
