#include "positionform.h"
#include "ui_positionform.h"

PositionForm::PositionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PositionForm)
{
    ui->setupUi(this);
}

PositionForm::~PositionForm()
{
    delete ui;
}

void PositionForm::init(){

}

void PositionForm::shutdown(){

}
