#include "workingorderform.h"
#include "ctp_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_workingorderform.h"

WorkingOrderForm::WorkingOrderForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::WorkingOrderForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "symbol"
               << "exchange"

               << "direction"
               << "offset"
               << "price"
               << "totalVolume"
               << "tradedVolume"
               << "status"

               << "insertDate"
               << "insertTime"
               << "cancelTime"

               << "bfOrderId";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

WorkingOrderForm::~WorkingOrderForm()
{
    delete ui;
}

void WorkingOrderForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotOrder, this, &WorkingOrderForm::onGotOrder);
}

void WorkingOrderForm::shutdown()
{
}

void WorkingOrderForm::onGotOrder(const BfOrderData& newOrder)
{
    // 全部成交或者撤销的，剔除=
    QString newKey = newOrder.bforderid().c_str();
    if (newOrder.status() == STATUS_ALLTRADED || newOrder.status() == STATUS_CANCELLED) {
        if (orders_.contains(newKey)) {
            orders_.remove(newKey);
        }
    } else {
        orders_[newKey] = newOrder;
    }

    // 更新界面=
    table_row_.clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(orders_.size());
    QStringList keys = orders_.keys();
    keys.sort();
    for (int i = 0; i < keys.length(); i++) {
        QString id = keys.at(i);
        table_row_[id] = i;
        QTableWidgetItem* item = new QTableWidgetItem(id);
        ui->tableWidget->setItem(i, table_col_.indexOf("bfOrderId"), item);
    }

    for (auto order : orders_) {
        QVariantMap vItem;
        vItem.insert("symbol", order.symbol().c_str());
        vItem.insert("exchange", order.exchange().c_str());

        vItem.insert("direction", CtpUtils::formatDirection(order.direction()));
        vItem.insert("offset", CtpUtils::formatOffset(order.offset()));
        vItem.insert("price", order.price());
        vItem.insert("totalVolume", order.totalvolume());
        vItem.insert("tradedVolume", order.tradedvolume());
        vItem.insert("status", CtpUtils::formatStatus(order.status()));

        vItem.insert("insertDate", order.insertdate().c_str());
        vItem.insert("insertTime", order.inserttime().c_str());
        vItem.insert("cancelTime", order.canceltime().c_str());

        vItem.insert("bfOrderId", order.bforderid().c_str());

        //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
        QString id = vItem.value("bfOrderId").toString();
        int row = table_row_.value(id);
        for (int i = 0; i < table_col_.count(); i++) {
            QVariant raw_val = vItem.value(table_col_.at(i));
            QString str_val = raw_val.toString();
            if (raw_val.type() == QMetaType::Double || raw_val.type() == QMetaType::Float) {
                str_val = QString().sprintf("%6.3f", raw_val.toDouble());
            }

            QTableWidgetItem* item = new QTableWidgetItem(str_val);
            ui->tableWidget->setItem(row, i, item);
        }
    }
}

void WorkingOrderForm::on_pushButtonQueryOrders_clicked()
{
    QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryOrders", Qt::QueuedConnection);
}

void WorkingOrderForm::on_pushButtonCancelAll_clicked()
{
    for (auto order : orders_) {
        BfCancelOrderReq req;
        req.set_symbol(order.symbol());
        req.set_exchange(order.exchange());
        req.set_bforderid(order.bforderid());

        QMetaObject::invokeMethod(g_sm->ctpMgr(), "cancelOrder", Qt::QueuedConnection, Q_ARG(BfCancelOrderReq, req));
    }
}
