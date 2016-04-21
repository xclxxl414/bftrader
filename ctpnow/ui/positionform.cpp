#include "positionform.h"
#include "ctp_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_positionform.h"

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
               << "price"
               << "frozen"
               << "ydPosition";
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
    void* contract = g_sm->ctpMgr()->getContract(symbol);
    if (contract) {
        exchange = CtpUtils::getExchangeFromContract(contract);
        volumeMultiple = CtpUtils::getVolumeMultipleFromContract(contract);
    }

    QVariantMap vItem;
    vItem.insert("symbol", symbol);
    vItem.insert("exchange", exchange);

    vItem.insert("direction", CtpUtils::formatDirection(pos.direction()));
    vItem.insert("position", pos.position());
    vItem.insert("price", pos.price() / volumeMultiple);
    vItem.insert("frozen", pos.frozen());
    vItem.insert("ydPosition", pos.ydposition());

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
