#include "tradeform.h"
#include "ui_tradeform.h"

TradeForm::TradeForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TradeForm)
{
    ui->setupUi(this);
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
