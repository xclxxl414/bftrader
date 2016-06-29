#include "tradeform.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_tradeform.h"

TradeForm::TradeForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TradeForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "symbol"
               << "exchange"

               << "direction"
               << "offset"
               << "price"
               << "volume"

               << "tradeDate"
               << "tradeTime"

               << "tradeId"
               << "bfOrderId";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

TradeForm::~TradeForm()
{
    delete ui;
}

void TradeForm::init()
{
}

void TradeForm::shutdown()
{
}
