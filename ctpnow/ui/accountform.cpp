#include "accountform.h"
#include "ui_accountform.h"

AccountForm::AccountForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountForm)
{
    ui->setupUi(this);
}

AccountForm::~AccountForm()
{
    delete ui;
}
