#include "barform.h"
#include "dbservice.h"
#include "leveldb/db.h"
#include "proto_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_barform.h"

BarForm::BarForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BarForm)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/datafeed.png"));

    //设置列=
    table_col_ << "symbol"
               << "exchange"
               << "period"

               << "actionDate"
               << "barTime"
               << "volume"
               << "openInterest"
               << "lastVolume"

               << "openPrice"
               << "highPrice"
               << "lowPrice"
               << "closePrice"

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

BarForm::~BarForm()
{
    delete ui;
}

void BarForm::init(QString symbol, QString exchange, int period)
{
    symbol_ = symbol;
    exchange_ = exchange;
    period_ = period;

    this->setWindowTitle(QString("history-bar-") + symbol_ + QString("-") + getPeriod());
    on_first128_clicked();
}

QString BarForm::getPeriod()
{
    return ProtoUtils::formatPeriod((BfBarPeriod)period_);
}

void BarForm::on_first128_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是bar-symbol-exchange-period+
    //最后一个是bar-symbol-exchange-period=
    QString key = QString().sprintf("bar-%s-%s-%s+", qPrintable(symbol_), qPrintable(exchange_), qPrintable(getPeriod()));

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
        BfBarData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.closeprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void BarForm::on_next128_clicked()
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

    //第一个是bar-symbol-exchange-period+
    //最后一个是bar-symbol-exchange-period=
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
        BfBarData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.closeprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void BarForm::on_pre128_clicked()
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

    //第一个是bar-symbol-exchange-period+
    //最后一个是bar-symbol-exchange-period=
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
        BfBarData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.closeprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void BarForm::on_last128_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是bar-symbol-exchange-period+
    //最后一个是bar-symbol-exchange-period=
    QString key = QString().sprintf("bar-%s-%s-%s=", qPrintable(symbol_), qPrintable(exchange_), qPrintable(getPeriod()));

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
        BfBarData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray error");
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }

        count++;
        x_.append(count);
        y_.append(bfItem.closeprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void BarForm::on_seekButton_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是bar-symbol-exchange-period+
    //最后一个是bar-symbol-exchange-period=
    QString key = ui->lineEdit->text();

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    QString msg("not found,input format:\nbar-IF1511-CFFEX-\nbar-IF1511-CFFEX-m1-20151023-1304\nbar-IF1511-CFFEX-m1-20151023-13:04:00");
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
        BfBarData bfItem;
        if (!bfItem.ParseFromArray(buf, len)) {
            break;
        }
        if (bfItem.symbol().length() == 0) {
            break;
        }
        count++;
        x_.append(count);
        y_.append(bfItem.closeprice());
        onGotTick(QString::fromStdString(it->key().ToString()), bfItem);
    }
    delete it;
    drawGraph();
}

void BarForm::on_delButton_clicked()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
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

void BarForm::onGotTick(QString key, const BfBarData& bfItem)
{
    QVariantMap vItem;
    vItem.insert("symbol", bfItem.symbol().c_str());
    vItem.insert("exchange", bfItem.exchange().c_str());
    vItem.insert("period", ProtoUtils::formatPeriod(bfItem.period()));

    vItem.insert("actionDate", bfItem.actiondate().c_str());
    vItem.insert("tickTime", bfItem.bartime().c_str());
    vItem.insert("volume", bfItem.volume());
    vItem.insert("openInterest", bfItem.openinterest());
    vItem.insert("lastVolume", bfItem.lastvolume());

    vItem.insert("openPrice", bfItem.openprice());
    vItem.insert("highPrice", bfItem.highprice());
    vItem.insert("lowPrice", bfItem.lowprice());
    vItem.insert("closePrice", bfItem.closeprice());

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

void BarForm::initGraph()
{
    ui->tickPlot->addGraph();
}

void BarForm::drawGraph()
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

void BarForm::on_tableWidget_cellClicked(int row, int column)
{
    QString key = ui->tableWidget->item(row, table_col_.indexOf("key"))->text();

    ui->lineEdit->setText(key);
}
