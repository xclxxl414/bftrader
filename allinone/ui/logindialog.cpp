#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("login");
    setWindowIcon(QIcon(":/images/heart.png"));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::getPassword()
{
    return ui->password->text();
}
