#include "historytickform.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "dbservice.h"
#include "leveldb/db.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_historytickform.h"

HistoryTickForm::HistoryTickForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistoryTickForm)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/heart.png"));

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
               << "lowerLimit"
               << "key";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }
    //this->ui->tableWidget->setColumnWidth(0,250);

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);

    //设置上下分割=
    //ui->splitter->setStretchFactor(0, 2);
    //ui->splitter->setStretchFactor(1, 1);

    //设置graph
    initGraph();
}

HistoryTickForm::~HistoryTickForm()
{
    delete ui;
}

void HistoryTickForm::init(QString symbol, QString exchange)
{
    symbol_ = symbol;
    exchange_ = exchange;

    this->setWindowTitle(QString("history-tick-" + symbol_));
    on_first128_clicked();
}

void HistoryTickForm::on_first128_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是tick-symbol-exchange+
    //最后一个是tick-symbol-exchange=
    QString key = QString().sprintf("tick-%s-%s+",qPrintable(symbol_),qPrintable(exchange_));

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    it->Seek(leveldb::Slice(key.toStdString()));
    if (it->Valid()) {
        it->Next();
    }
    int count = 0;
    x_.clear();
    y_.clear();
    for (; it->Valid() && count < 128; it->Next()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfTickData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.lastprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void HistoryTickForm::on_next128_clicked()
{
    if (ui->tableWidget->rowCount() == 0) {
        return;
    }

    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是tick-symbol-exchange+
    //最后一个是tick-symbol-exchange=
    int r = ui->tableWidget->rowCount() - 1;
    QString key = ui->tableWidget->item(r, table_col_.indexOf("key"))->text();

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    it->Seek(leveldb::Slice(key.toStdString()));
    if (it->Valid()) {
        it->Next();
    }
    int count = 0;
    x_.clear();
    y_.clear();
    for (; it->Valid() && count < 128; it->Next()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfTickData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.lastprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void HistoryTickForm::on_pre128_clicked()
{
    if (ui->tableWidget->rowCount() == 0) {
        return;
    }

    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是tick-symbol-exchange+
    //最后一个是tick-symbol-exchange=
    int r = ui->tableWidget->rowCount() - 1;
    QString key = ui->tableWidget->item(r, table_col_.indexOf("key"))->text();

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    it->Seek(leveldb::Slice(key.toStdString()));
    if (it->Valid()) {
        it->Prev();
    }
    int count = 0;
    x_.clear();
    y_.clear();
    for (; it->Valid() && count < 128; it->Prev()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfTickData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.lastprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void HistoryTickForm::on_last128_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是tick-symbol-exchange+
    //最后一个是tick-symbol-exchange=
    QString key = QString().sprintf("tick-%s-%s=", qPrintable(symbol_), qPrintable(exchange_));

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    it->Seek(leveldb::Slice(key.toStdString()));
    if (it->Valid()) {
        it->Prev();
    }
    int count = 0;
    x_.clear();
    y_.clear();
    for (; it->Valid() && count < 128; it->Prev()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfTickData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.lastprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void HistoryTickForm::on_seekButton_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是tick-id+
    //最后一个是tick-id=
    QString key = ui->lineEdit->text();

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    QString msg("not found,input format:\ntick--CFFEX-IF1511-\ntick-CFFEX-IF1511-20151023-1304\ntick-CFFEX-IF1511-20151023-13:04:00.000");
    it->Seek(leveldb::Slice(key.toStdString()));
    if (!it->Valid()) {
        BfError(msg);
    }
    int count = 0;
    x_.clear();
    y_.clear();
    for (; it->Valid() && count < 128; it->Prev()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfTickData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }
        count++;
        x_.append(count);
        y_.append(bfItem.lastprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void HistoryTickForm::on_delButton_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    ;
    QString key = ui->lineEdit->text();
    leveldb::ReadOptions options;
    std::string val;
    leveldb::Status status = db->Get(options, key.toStdString(), &val);
    if (status.ok()) {
        leveldb::WriteOptions options;
        status = db->Delete(options, key.toStdString());
    }

    if (status.ok()) {
        BfInfo("del,ok");
    } else {
        QString errStr = QString::fromStdString(status.ToString());
        BfError(errStr);
    }
}

void HistoryTickForm::onGotTick(QString key, const BfTickData& bfTick)
{
    QVariantMap vItem;
    vItem.insert("symbol", bfTick.symbol().c_str());
    // tick里面的exchange不一定有=
    QString exchange = bfTick.exchange().c_str();
    if (exchange.trimmed().length() == 0) {
        exchange = exchange_;
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

    vItem.insert("key", key);

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

void HistoryTickForm::initGraph()
{
    ui->tickPlot->addGraph();
}

void HistoryTickForm::drawGraph()
{
    ui->tickPlot->graph()->setData(x_, y_);
    ui->tickPlot->graph()->rescaleAxes(false);
    ui->tickPlot->xAxis->scaleRange(1.1, ui->tickPlot->xAxis->range().center());
    ui->tickPlot->yAxis->scaleRange(1.1, ui->tickPlot->yAxis->range().center());
    ui->tickPlot->xAxis->setTicks(true);
    ui->tickPlot->yAxis->setTicks(true);
    ui->tickPlot->axisRect()->setupFullAxesBox();
    ui->tickPlot->replot();
}

void HistoryTickForm::on_tableWidget_cellClicked(int row, int column)
{
    QString key = ui->tableWidget->item(row, table_col_.indexOf("key"))->text();

    ui->lineEdit->setText(key);
}
