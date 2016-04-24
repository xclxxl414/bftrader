#include "finishedorderform.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_finishedorderform.h"

namespace {

QString formatDirection(BfDirection direction)
{
    switch (direction) {
    case DIRECTION_LONG:
        return "long";
    case DIRECTION_SHORT:
        return "short";
    case DIRECTION_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid directioni");
    }

    return "unknown";
}

QString formatOffset(BfOffset offset)
{
    switch (offset) {
    case OFFSET_CLOSE:
        return "close";
    case OFFSET_CLOSETODAY:
        return "closetoday";
    case OFFSET_CLOSEYESTERDAY:
        return "closeyesterday";
    case OFFSET_OPEN:
        return "open";
    case OFFSET_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid offset");
    }

    return "unknown";
}

QString formatStatus(BfStatus status)
{
    switch (status) {
    case STATUS_ALLTRADED:
        return "alltraded";
    case STATUS_CANCELLED:
        return "cancelled";
    case STATUS_NOTTRADED:
        return "nottraced";
    case STATUS_PARTTRADED:
        return "parttraded";
    case STATUS_UNKNOWN:
        return "unknown";
    default:
        qFatal("invalid status");
    }

    return "unknown";
}
}

FinishedOrderForm::FinishedOrderForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FinishedOrderForm)
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

FinishedOrderForm::~FinishedOrderForm()
{
    delete ui;
}

void FinishedOrderForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotOrder, this, &FinishedOrderForm::onGotOrder);
}

void FinishedOrderForm::shutdown()
{
}

void FinishedOrderForm::onGotOrder(const BfOrderData& newOrder)
{
    // 全部成交或者撤销的=
    QString newKey = newOrder.bforderid().c_str();
    if (newOrder.status() == STATUS_ALLTRADED || newOrder.status() == STATUS_CANCELLED) {
        orders_[newKey] = newOrder;
    }

    // 更新界面=
    table_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(orders_.size());
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

        vItem.insert("direction", formatDirection(order.direction()));
        vItem.insert("offset", formatOffset(order.offset()));
        vItem.insert("price", order.price());
        vItem.insert("totalVolume", order.totalvolume());
        vItem.insert("tradedVolume", order.tradedvolume());
        vItem.insert("status", formatStatus(order.status()));

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
