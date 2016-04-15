#include "pendingorderform.h"
#include "ui_pendingorderform.h"

PendingOrderForm::PendingOrderForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PendingOrderForm)
{
    ui->setupUi(this);
}

PendingOrderForm::~PendingOrderForm()
{
    delete ui;
}

void PendingOrderForm::init(){

}

void PendingOrderForm::shutdown(){

}
