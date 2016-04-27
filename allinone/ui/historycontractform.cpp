#include "historycontractform.h"
#include "ctp_utils.h"
#include "dbservice.h"
#include "encode_utils.h"
#include "historytickform.h"
#include "leveldb/db.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_historycontractform.h"

HistoryContractForm::HistoryContractForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistoryContractForm)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/heart.png"));

    //设置列=
    table_col_ << "symbol"
               << "exchange"
               << "name"

               << "productClass"
               << "volumeMultiple"
               << "priceTick"

               << "maxLimit"
               << "minLimit"
               << "maxMarket"
               << "minMarket"
               << "key";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

HistoryContractForm::~HistoryContractForm()
{
    delete ui;
}

void HistoryContractForm::init()
{
    this->setWindowTitle(QStringLiteral("history-contract"));
    refresh();
}

void HistoryContractForm::on_refreshButton_clicked()
{
    refresh();
}

void HistoryContractForm::refresh()
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

        onGotContract(QString::fromStdString(it->key().ToString()), bfContract);
    }
    delete it;
}

void HistoryContractForm::onGotContract(QString key, const BfContractData& bfItem)
{
    QVariantMap vItem;
    vItem.insert("symbol", bfItem.symbol().c_str());
    vItem.insert("exchange", bfItem.exchange().c_str());
    vItem.insert("name", bfItem.name().c_str());

    vItem.insert("productClass", CtpUtils::formatProduct(bfItem.productclass()));
    vItem.insert("volumeMultiple", bfItem.volumemultiple());
    vItem.insert("priceTick", bfItem.pricetick());

    vItem.insert("maxLimit", bfItem.maxlimit());
    vItem.insert("minLimit", bfItem.minlimit());
    vItem.insert("maxMarket", bfItem.maxmarket());
    vItem.insert("minMarket", bfItem.minmartet());

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

void HistoryContractForm::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QString symbol = ui->tableWidget->item(row, table_col_.indexOf("symbol"))->text();
    QString exchange = ui->tableWidget->item(row, table_col_.indexOf("exchange"))->text();

    HistoryTickForm* form = new HistoryTickForm();
    form->setWindowFlags(Qt::Window);
    form->init(symbol, exchange);
    centerWindow(form);
    form->show();
}

void HistoryContractForm::on_tableWidget_cellClicked(int row, int column)
{
    QString symbol = ui->tableWidget->item(row, table_col_.indexOf("symbol"))->text();
    QString exchange = ui->tableWidget->item(row, table_col_.indexOf("exchange"))->text();

    ui->lineEditSymbol->setText(symbol);
    ui->lineEditExchange->setText(exchange);
}

void HistoryContractForm::on_pushButtonTick_clicked()
{
    QString symbol = ui->lineEditSymbol->text();
    QString exchange = ui->lineEditExchange->text();

    if(symbol.length()!=0 && exchange.length()!=0){
        HistoryTickForm* form = new HistoryTickForm();
        form->setWindowFlags(Qt::Window);
        form->init(symbol, exchange);
        centerWindow(form);
        form->show();
    }
}
