#include "tickform.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "ringbufferform.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_tickform.h"

TickForm::TickForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TickForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "symbol"
               << "exchange"

               << "actionDate"
               << "tickTime"
               << "lastPrice"

               << "bidPrice1"
               << "askPrice1"
               << "bidVolume1"
               << "askVolume1"

               << "volume"
               << "openInterest"
               << "lastVolume"

               << "openPrice"
               << "highPrice"
               << "lowPrice"
               << "preClosePrice"
               << "upperLimit"
               << "lowerLimit";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

TickForm::~TickForm()
{
    delete ui;
}

void TickForm::init()
{
    // ctpmgr
    // tablewidget更新ui太慢了(是自适应高度和宽度搞的，去掉自适应后好了)，不过改成500毫秒的定时器也不错=
    //QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTick, this, &TickForm::onGotTick);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotContracts, this, &TickForm::onGotContracts);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &TickForm::onTradeWillBegin);

    this->updateTickTimer_ = new QTimer(this);
    this->updateTickTimer_->setInterval(500);
    QObject::connect(this->updateTickTimer_, &QTimer::timeout, this, &TickForm::onUpdateTick);
}

void TickForm::shutdown()
{
    this->updateTickTimer_->stop();
    delete this->updateTickTimer_;
    this->updateTickTimer_ = nullptr;
}

void TickForm::onGotTick(void* curTick, void* preTick)
{
    BfTickData bfTick;
    CtpUtils::translateTick(curTick, preTick, &bfTick);

    QVariantMap vItem;
    vItem.insert("symbol", bfTick.symbol().c_str());
    // tick里面的exchange不一定有=
    QString exchange = bfTick.exchange().c_str();
    if (exchange.trimmed().length() == 0) {
        void* contract = g_sm->ctpMgr()->getContract(bfTick.symbol().c_str());
        if (contract) {
            exchange = CtpUtils::getExchangeFromContract(contract);
        }
    }
    vItem.insert("exchange", exchange);
    vItem.insert("actionDate", bfTick.actiondate().c_str());
    vItem.insert("tickTime", bfTick.ticktime().c_str());
    vItem.insert("lastPrice", bfTick.lastprice());
    vItem.insert("volume", bfTick.volume());
    vItem.insert("openInterest", bfTick.openinterest());
    vItem.insert("lastVolume", bfTick.lastvolume());

    vItem.insert("bidPrice1", bfTick.bidprice1());
    vItem.insert("askPrice1", bfTick.askprice1());
    vItem.insert("bidVolume1", bfTick.bidvolume1());
    vItem.insert("askVolume1", bfTick.askvolume1());

    vItem.insert("openPrice", bfTick.openprice());
    vItem.insert("highPrice", bfTick.highprice());
    vItem.insert("lowPrice", bfTick.lowprice());
    vItem.insert("preClosePrice", bfTick.precloseprice());
    vItem.insert("upperLimit", bfTick.upperlimit());
    vItem.insert("lowerLimit", bfTick.lowerlimit());

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    QString id = vItem.value("symbol").toString();
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

void TickForm::onGotContracts(QStringList ids, QStringList idsAll)
{
    //设置行，按排序后合约来，一个合约一行=
    table_row_.clear();
    QStringList sorted_ids = ids;
    sorted_ids.sort();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(sorted_ids.length());
    for (int i = 0; i < sorted_ids.length(); i++) {
        QString id = sorted_ids.at(i);
        table_row_[id] = i;
        QTableWidgetItem* item = new QTableWidgetItem(id);
        ui->tableWidget->setItem(i, table_col_.indexOf("symbol"), item);

        //设置exchange
        void* contract = g_sm->ctpMgr()->getContract(id);
        if (contract) {
            QString exchange = CtpUtils::getExchangeFromContract(contract);
            QTableWidgetItem* item = new QTableWidgetItem(exchange);
            ui->tableWidget->setItem(i, table_col_.indexOf("exchange"), item);
        }
    }

    this->updateTickTimer_->start();
}

void TickForm::onTradeWillBegin()
{
    this->updateTickTimer_->stop();

    table_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(0);
}

void TickForm::onUpdateTick()
{
    for (int i = 0; i < table_row_.size(); i++) {
        QString id = table_row_.key(i);
        void* curTick = g_sm->ctpMgr()->getLatestTick(id);
        void* preTick = g_sm->ctpMgr()->getPreLatestTick(id);
        if (curTick) {
            onGotTick(curTick, preTick);
        }
    }
}

void TickForm::on_pushButtonSendOrder_clicked()
{
    QString symbol = ui->lineEditSymbol->text();
    QString exchange = ui->lineEditExchange->text();
    double price = ui->doubleSpinBoxPrice->value();
    int volume = ui->spinBoxVolume->value();
    BfDirection direction = (BfDirection)(ui->comboBoxDirection->currentIndex() + 1);
    BfOffset offset = (BfOffset)(ui->comboBoxOffset->currentIndex() + 1);
    BfPriceType priceType = (BfPriceType)(ui->comboBoxPriceType->currentIndex() + 1);

    BfSendOrderReq req;
    req.set_symbol(symbol.toStdString());
    req.set_exchange(exchange.toStdString());
    req.set_price(price);
    req.set_volume(volume);
    req.set_direction(direction);
    req.set_offset(offset);
    req.set_pricetype(priceType);

    QMetaObject::invokeMethod(g_sm->ctpMgr(), "sendOrder", Qt::QueuedConnection, Q_ARG(BfSendOrderReq, req));
}

void TickForm::on_tableWidget_cellDoubleClicked(int row, int column)
{
    (void)column;

    QString symbol = ui->tableWidget->item(row, table_col_.indexOf("symbol"))->text();
    QString exchange = ui->tableWidget->item(row, table_col_.indexOf("exchange"))->text();

    RingBufferForm* form = new RingBufferForm();
    form->setWindowFlags(Qt::Window);
    form->init(symbol, exchange);
    centerWindow(form);
    form->show();
}

void TickForm::on_tableWidget_cellClicked(int row, int column)
{
    (void)column;

    QString symbol = ui->tableWidget->item(row, table_col_.indexOf("symbol"))->text();
    QString exchange = ui->tableWidget->item(row, table_col_.indexOf("exchange"))->text();
    auto item = ui->tableWidget->item(row, table_col_.indexOf("lastPrice"));
    double lastPrice = item ? item->text().toDouble() : 0.0;
    item = ui->tableWidget->item(row, table_col_.indexOf("upperLimit"));
    double upperLimit = item ? item->text().toDouble() : 0.0;
    item = ui->tableWidget->item(row, table_col_.indexOf("lowerLimit"));
    double lowerLimit = item ? item->text().toDouble() : 0.0;

    ui->lineEditSymbol->setText(symbol);
    ui->lineEditExchange->setText(exchange);

    //设置price
    ui->doubleSpinBoxPrice->setValue(lastPrice);
    ui->doubleSpinBoxPrice->setMaximum(upperLimit);
    ui->doubleSpinBoxPrice->setMinimum(lowerLimit);

    void* contract = g_sm->ctpMgr()->getContract(symbol);
    if (contract) {
        BfContractData bfContract;
        CtpUtils::translateContract(contract, &bfContract);
        ui->doubleSpinBoxPrice->setSingleStep(bfContract.pricetick());

        // 设置volume
        ui->spinBoxVolume->setValue(bfContract.minlimit());
        ui->spinBoxVolume->setMaximum(bfContract.maxlimit());
        ui->spinBoxVolume->setMinimum(bfContract.minlimit());
        ui->spinBoxVolume->setSingleStep(bfContract.minlimit());
    }
}

void TickForm::on_pushButtonTick_clicked()
{
    QString symbol = ui->lineEditSymbol->text();
    QString exchange = ui->lineEditExchange->text();

    if(symbol.length()!=0 && exchange.length()!=0){
        RingBufferForm* form = new RingBufferForm();
        form->setWindowFlags(Qt::Window);
        form->init(symbol, exchange);
        centerWindow(form);
        form->show();
    }
}
