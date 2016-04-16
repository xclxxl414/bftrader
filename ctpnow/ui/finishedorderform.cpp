#include "finishedorderform.h"
#include "ui_finishedorderform.h"

FinishedOrderForm::FinishedOrderForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FinishedOrderForm)
{
    ui->setupUi(this);
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
