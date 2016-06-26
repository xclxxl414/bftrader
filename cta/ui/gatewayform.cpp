#include "gatewayform.h"
#include "gatewaymgr.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_gatewayform.h"

GatewayForm::GatewayForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::GatewayForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "gatewayId"
               << "ip"
               << "port"
               << "status";

    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);

    // buttons
    this->ui->pushButtonConnect->setEnabled(true);
    this->ui->pushButtonDisconnect->setEnabled(false);
}

GatewayForm::~GatewayForm()
{
    delete ui;
}

void GatewayForm::init()
{
}

void GatewayForm::shutdown()
{
}

void GatewayForm::on_pushButtonConnect_clicked()
{
    this->ui->pushButtonConnect->setEnabled(false);
    this->ui->pushButtonDisconnect->setEnabled(true);

    QString gatewayId = "ctpgateway";
    QString endpoint = "localhost:50051";
    BfConnectPushReq req;
    req.set_clientid("cta");
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "connectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId), Q_ARG(QString, endpoint), Q_ARG(BfConnectPushReq, req));
}

void GatewayForm::on_pushButtonDisconnect_clicked()
{
    this->ui->pushButtonConnect->setEnabled(true);
    this->ui->pushButtonDisconnect->setEnabled(false);

    QString gatewayId = "ctpgateway";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "disconnectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId));
}
