#include "accountform.h"
#include "ctpmgr.h"
#include "servicemgr.h"
#include "ui_accountform.h"

AccountForm::AccountForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AccountForm)
{
    ui->setupUi(this);
}

AccountForm::~AccountForm()
{
    delete ui;
}

void AccountForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotAccount, this, &AccountForm::onGotAccount);
}

void AccountForm::shutdown()
{
}

QString AccountForm::formatDouble(double val)
{
    return QString().sprintf("%6.1f", val);
}

void AccountForm::onGotAccount(double balance, double available, double frozenMargin, double closeProfit, double positionProfit)
{
    ui->lineEditBalance->setText(formatDouble(balance));
    ui->lineEditAvailable->setText(formatDouble(available));
    ui->lineEditFrozenMargin->setText(formatDouble(frozenMargin) + "%");
    ui->lineEditCloseProfit->setText(formatDouble(closeProfit));
    ui->lineEditPositionProfit->setText(formatDouble(positionProfit));
}

void AccountForm::on_pushButtonQueryAccount_clicked()
{
    QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryAccount", Qt::QueuedConnection);
}
