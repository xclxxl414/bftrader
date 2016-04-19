#include "positionform.h"
#include "nofocusdelegate.h"
#include "ui_positionform.h"

PositionForm::PositionForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PositionForm)
{
    ui->setupUi(this);

    //设置列=
    instruments_col_ << "symbol"
                     << "exchange"

                     << "direction"
                     << "position"
                     << "frozen"
                     << "price";
    this->ui->tableWidget->setColumnCount(instruments_col_.length());
    for (int i = 0; i < instruments_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(instruments_col_.at(i)));
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
