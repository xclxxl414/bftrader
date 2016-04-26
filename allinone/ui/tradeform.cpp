#include "tradeform.h"
#include "ctp_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_tradeform.h"

TradeForm::TradeForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TradeForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "symbol"
               << "exchange"

               << "direction"
               << "offset"
               << "price"
               << "volume"

               << "tradeDate"
               << "tradeTime"

               << "tradeId"
               << "bfOrderId";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

TradeForm::~TradeForm()
{
    delete ui;
}

void TradeForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTrade, this, &TradeForm::onGotTrade);
}

void TradeForm::shutdown()
{
}

void TradeForm::onGotTrade(const BfTradeData& trade)
{
    QVariantMap vItem;
    vItem.insert("symbol", trade.symbol().c_str());
    vItem.insert("exchange", trade.exchange().c_str());

    vItem.insert("direction", CtpUtils::formatDirection(trade.direction()));
    vItem.insert("offset", CtpUtils::formatOffset(trade.offset()));
    vItem.insert("price", trade.price());
    vItem.insert("volume", trade.volume());
    vItem.insert("tradeDate", trade.tradedate().c_str());
    vItem.insert("tradeTime", trade.tradetime().c_str());

    vItem.insert("tradeId", trade.tradeid().c_str());
    vItem.insert("bfOrderId", trade.bforderid().c_str());

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
