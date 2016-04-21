#include "positionform.h"
#include "ThostFtdcUserApiStruct.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_positionform.h"

namespace {

QString formatDirection(BfDirection direction)
{
    switch (direction) {
    case DIRECTION_NET:
        return "net";
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

PositionForm::PositionForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PositionForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "symbol"
               << "exchange"

               << "direction"
               << "position"
               << "ydPosition"
               << "frozen"
               << "price";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

PositionForm::~PositionForm()
{
    delete ui;
}

void PositionForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotPosition, this, &PositionForm::onGotPosition);
}

void PositionForm::shutdown()
{
}

void PositionForm::onGotPosition(const BfPositionData& pos)
{
    if (pos.position() == 0 && pos.frozen() == 0) {
        return;
    }

    QString symbol = QString::fromStdString(pos.symbol());
    QString exchange = QString::fromStdString(pos.exchange());
    int volumeMultiple = 1;
    auto contract = (CThostFtdcInstrumentField*)g_sm->ctpMgr()->getContract(symbol);
    if (contract) {
        exchange = contract->ExchangeID;
        volumeMultiple = contract->VolumeMultiple;
    }

    QVariantMap vItem;
    vItem.insert("symbol", symbol);
    vItem.insert("exchange", exchange);
    vItem.insert("direction", formatDirection(pos.direction()));
    vItem.insert("position", pos.position());
    vItem.insert("ydPosition", pos.ydposition());
    vItem.insert("frozen", pos.frozen());
    vItem.insert("price", pos.price() / volumeMultiple);

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
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

void PositionForm::on_pushButtonQueryPosition_clicked()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryPosition", Qt::QueuedConnection);
}
