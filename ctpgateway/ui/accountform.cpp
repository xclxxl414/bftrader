#include "accountform.h"
#include "gatewaymgr.h"
#include "servicemgr.h"
#include "ui_accountform.h"
#include <QDesktopServices>
#include <QUrl>

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
    // gatewaymgr
    QObject::connect(g_sm->gatewayMgr(), &GatewayMgr::gotAccount, this, &AccountForm::onGotAccount);
}

void AccountForm::shutdown()
{
}

QString AccountForm::formatDouble(double val)
{
    return QString().sprintf("%6.1f", val);
}

void AccountForm::onGotAccount(const BfAccountData& account)
{
    ui->lineEditBalance->setText(formatDouble(account.balance()));
    ui->lineEditAvailable->setText(formatDouble(account.available()));
    ui->lineEditFrozenMargin->setText(formatDouble(account.frozenmargin()) + "%");
    ui->lineEditCloseProfit->setText(formatDouble(account.closeprofit()));
    ui->lineEditPositionProfit->setText(formatDouble(account.positionprofit()));
}

void AccountForm::on_pushButtonQueryAccount_clicked()
{
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "queryAccount", Qt::QueuedConnection);
}

void AccountForm::on_pushButtonFeedback_clicked()
{
    QUrl url("https://github.com/sunwangme/bftrader/issues");
    QDesktopServices::openUrl(url);
}
