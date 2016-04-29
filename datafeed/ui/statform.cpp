#include "statform.h"
#include "barform.h"
#include "ctpmgr.h"
#include "dbservice.h"
#include "leveldb/db.h"
#include "proto_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "tickform.h"
#include "ui_statform.h"

StatForm::StatForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatForm)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/datafeed.png"));

    //设置列=
    table_col_ << "symbol"
               << "exchange"
               << "name"

               << "period"
               << "startDate"
               << "startTime"
               << "endDate"
               << "endTime";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

StatForm::~StatForm()
{
    delete ui;
}

void StatForm::init()
{
    this->setWindowTitle(QStringLiteral("history-stat"));
    refresh();
}

void StatForm::shutdown()
{
}

void StatForm::refresh()
{
    leveldb::DB* db = g_sm->dbService()->getDb();
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是contract+
    //最后一个是contract=
    QString key;
    key = QStringLiteral("contract+");

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    it->Seek(leveldb::Slice(key.toStdString()));
    if (it->Valid()) {
        it->Next();
    }
    for (; it->Valid(); it->Next()) {
        //遇到了前后两个结束item
        const char* buf = it->value().data();
        int len = it->value().size();
        BfContractData bfContract;
        //std::string stdKey = it->key().ToString();
        //std::string stdVal = it->value().ToString();
        //if(!bfContract.ParseFromString(stdVal)){
        if (!bfContract.ParseFromArray(buf, len)) {
            qFatal("ParseFromArray fail");
            break;
        }
        if (bfContract.symbol().length() == 0) {
            break;
        }

        // 是否有tick/bar=
        statTick(bfContract.symbol().c_str(), bfContract.exchange().c_str(), bfContract.name().c_str());
        statBar(bfContract.symbol().c_str(), bfContract.exchange().c_str(), bfContract.name().c_str());
    }
    delete it;
}

void StatForm::statTick(QString symbol, QString exchange, QString name)
{
    QString startDate, startTime, endDate, endTime;

    if (1) {
        leveldb::DB* db = g_sm->dbService()->getDb();
        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        //第一个是tick-symbol-exchange+
        //最后一个是tick-symbol-exchange=
        QString key = QString().sprintf("tick-%s-%s+", qPrintable(symbol), qPrintable(exchange));
        it->Seek(leveldb::Slice(key.toStdString()));
        if (it->Valid()) {
            it->Next();
        }
        if (it->Valid()) {
            //遇到了前后两个结束item
            const char* buf = it->value().data();
            int len = it->value().size();
            BfTickData bfItem;
            if (bfItem.ParseFromArray(buf, len) && bfItem.symbol().length() != 0) {
                startDate = bfItem.actiondate().c_str();
                startTime = bfItem.ticktime().c_str();
            }
        }
        delete it;
    }

    if (startDate.length() != 0) {
        leveldb::DB* db = g_sm->dbService()->getDb();
        leveldb::ReadOptions options;
        options.fill_cache = false;
        leveldb::Iterator* it = db->NewIterator(options);
        if (!it) {
            qFatal("NewIterator == nullptr");
        }

        //第一个是tick-symbol-exchange+
        //最后一个是tick-symbol-exchange=
        QString key = QString().sprintf("tick-%s-%s=", qPrintable(symbol), qPrintable(exchange));
        it->Seek(leveldb::Slice(key.toStdString()));
        if (it->Valid()) {
            it->Prev();
        }
        if (it->Valid()) {
            //遇到了前后两个结束item
            const char* buf = it->value().data();
            int len = it->value().size();
            BfTickData bfItem;
            if (bfItem.ParseFromArray(buf, len) && bfItem.symbol().length() != 0) {
                endDate = bfItem.actiondate().c_str();
                endTime = bfItem.ticktime().c_str();
            }
        }
        delete it;
    }

    if (startDate.length() != 0 && endDate.length() != 0) {
        onGotData(symbol, exchange, name, "tick", startDate, startTime, endDate, endTime);
    }
}

void StatForm::statBar(QString symbol, QString exchange, QString name)
{
}

void StatForm::onGotData(QString symbol, QString exchange, QString name, QString period, QString startDate, QString startTime, QString endDate, QString endTime)
{
    QVariantMap vItem;
    vItem.insert("symbol", symbol);
    vItem.insert("exchange", exchange);
    vItem.insert("name", name);

    vItem.insert("period", period);
    vItem.insert("startDate", startDate);
    vItem.insert("startTime", startTime);

    vItem.insert("endDate", endDate);
    vItem.insert("endTime", endTime);

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

void StatForm::on_pushButtonRefresh_clicked()
{
    refresh();
}

void StatForm::on_pushButtonShowData_clicked()
{
    QString symbol = ui->lineEditSymbol->text();
    QString exchange = ui->lineEditExchange->text();
    QString period = ui->lineEditPeriod->text();

    if (symbol.length() != 0 && exchange.length() != 0 && period.length() != 0) {
        if (period == "tick") {
            TickForm* form = new TickForm();
            form->setWindowFlags(Qt::Window);
            form->init(symbol, exchange);
            centerWindow(form);
            form->show();
        } else {
        }
    }
}

void StatForm::on_tableWidget_cellClicked(int row, int column)
{
    QString symbol = ui->tableWidget->item(row, table_col_.indexOf("symbol"))->text();
    QString exchange = ui->tableWidget->item(row, table_col_.indexOf("exchange"))->text();
    QString period = ui->tableWidget->item(row, table_col_.indexOf("period"))->text();

    ui->lineEditSymbol->setText(symbol);
    ui->lineEditExchange->setText(exchange);
    ui->lineEditPeriod->setText(period);
}
