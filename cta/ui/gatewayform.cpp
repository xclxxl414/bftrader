#include "gatewayform.h"
#include "tablewidget_helper.h"
#include "ui_gatewayform.h"
#include "servicemgr.h"
#include "gatewaymgr.h"

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
    QString gatewayId = "ctpgateway";
    QString endpoint = "localhost:50051";
    BfConnectReq req;
    req.set_clientid("cta");
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "connectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId), Q_ARG(QString, endpoint), Q_ARG(BfConnectReq, req));
}

void GatewayForm::on_pushButtonDisconnect_clicked()
{
    QString gatewayId = "ctpgateway";
    QMetaObject::invokeMethod(g_sm->gatewayMgr(), "disconnectGateway", Qt::QueuedConnection, Q_ARG(QString, gatewayId));
}
