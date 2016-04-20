#include "InstrumentsForm.h"
#include "ThostFtdcUserApiStruct.h"
#include "encode_utils.h"
#include "ui_instrumentsform.h"
#include <leveldb/db.h>

InstrumentsForm::InstrumentsForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::InstrumentsForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "InstrumentID"
               << "ExchangeID"
               << "InstrumentName"
               << "ExchangeInstID"
               << "PriceTick"
               << "CreateDate"
               << "OpenDate"
               << "ExpireDate"
               << "StartDelivDate";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }
}

InstrumentsForm::~InstrumentsForm()
{
    delete ui;
}

void InstrumentsForm::init(leveldb::DB* db)
{
    this->setWindowTitle(QStringLiteral("instruments"));
    db_ = db;
    refresh();
}

void InstrumentsForm::on_refreshButton_clicked()
{
    refresh();
}

void InstrumentsForm::refresh()
{
    leveldb::DB* db = db_;
    leveldb::ReadOptions options;
    options.fill_cache = false;
    leveldb::Iterator* it = db->NewIterator(options);
    if (!it) {
        qFatal("NewIterator == nullptr");
    }

    //第一个是instrument+
    //最后一个是instrument=
    QString key;
    key = QStringLiteral("instrument+");

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    it->Seek(leveldb::Slice(key.toStdString()));
    if (it->Valid()) {
        it->Next();
    }
    for (; it->Valid(); it->Next()) {
        //遇到了前后两个结束item
        auto item = (CThostFtdcInstrumentField*)it->value().data();
        if (item->InstrumentID[0] == 0) {
            break;
        }
        if (it->value().size() != sizeof(CThostFtdcInstrumentField)) {
            qFatal("it->value().size() != sizeof(InstrumentField)");
        }
        onGotInstrument(item);
    }
    delete it;
}

void InstrumentsForm::onGotInstrument(void* p)
{
    auto item = (CThostFtdcInstrumentField*)p;

    QVariantMap mdItem;
    mdItem.insert("InstrumentID", item->InstrumentID);
    mdItem.insert("ExchangeID", item->ExchangeID);
    mdItem.insert("InstrumentName", gbk2utf16(item->InstrumentName));
    mdItem.insert("ExchangeInstID", item->ExchangeInstID);
    mdItem.insert("PriceTick", item->PriceTick);
    mdItem.insert("CreateDate", item->CreateDate);
    mdItem.insert("OpenDate", item->OpenDate);
    mdItem.insert("ExpireDate", item->ExpireDate);
    mdItem.insert("StartDelivDate", item->StartDelivDate);

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    for (int i = 0; i < table_col_.count(); i++) {
        QVariant raw_val = mdItem.value(table_col_.at(i));
        QString str_val = raw_val.toString();
        if (raw_val.type() == QMetaType::Double || raw_val.type() == QMetaType::Float) {
            str_val = QString().sprintf("%6.1f", raw_val.toDouble());
        }

        QTableWidgetItem* item = new QTableWidgetItem(str_val);
        ui->tableWidget->setItem(row, i, item);
    }
}
