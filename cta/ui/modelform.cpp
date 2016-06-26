#include "modelform.h"
#include "tablewidget_helper.h"
#include "ui_modelform.h"

ModelForm::ModelForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ModelForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "modelId"
               << "langType"
               << "path";

    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

ModelForm::~ModelForm()
{
    delete ui;
}

void ModelForm::init()
{
}

void ModelForm::shutdown()
{
}
