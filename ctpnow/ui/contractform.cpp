#include "contractform.h"
#include "ThostFtdcUserApiStruct.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "nofocusdelegate.h"
#include "servicemgr.h"
#include "ui_contractform.h"

ContractForm::ContractForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ContractForm)
{
    ui->setupUi(this);

    //设置列=
    instruments_col_ << "symbol"
                     << "exchange"
                     << "name"

                     << "productClass"
                     << "volumeMultiple"
                     << "priceTick"

                     << "maxLimit"
                     << "minLimit"
                     << "maxMarket"
                     << "minMarket";
    this->ui->tableWidget->setColumnCount(instruments_col_.length());
    for (int i = 0; i < instruments_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(instruments_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

ContractForm::~ContractForm()
{
    delete ui;
}

void ContractForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotInstruments, this, &ContractForm::onGotInstruments);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeClosed, this, &ContractForm::onTradeClosed);
}

void ContractForm::shutdown()
{
}

void ContractForm::onGotInstruments(QStringList ids)
{
    //设置行，按排序后合约来，一个合约一行=
    instruments_row_.clear();
    QStringList sorted_ids = ids;
    sorted_ids.sort();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(sorted_ids.length());
    for (int i = 0; i < sorted_ids.length(); i++) {
        QString id = sorted_ids.at(i);
        instruments_row_[id] = i;
        QTableWidgetItem* item = new QTableWidgetItem(id);
        ui->tableWidget->setItem(i, 0, item);
    }

    //设置行内容=
    for (int i = 0; i < sorted_ids.length(); i++) {
        QString id = sorted_ids.at(i);
        auto contract = (CThostFtdcInstrumentField*)g_sm->ctpMgr()->getContract(id);
        this->onGotContract(contract);
    }
}

void ContractForm::onGotContract(void* contract)
{
    auto pif = (CThostFtdcInstrumentField*)contract;

    QVariantMap ifItem;
    ifItem.insert("symbol", pif->InstrumentID);
    ifItem.insert("exchange", pif->ExchangeID);
    ifItem.insert("name", gbk2utf16(pif->InstrumentName));

    ifItem.insert("productClass", QString(pif->ProductClass));
    ifItem.insert("volumeMultiple", pif->VolumeMultiple);
    ifItem.insert("priceTick", pif->PriceTick);

    ifItem.insert("maxLimit", pif->MaxLimitOrderVolume);
    ifItem.insert("minLimit", pif->MinMarketOrderVolume);
    ifItem.insert("maxMarket", pif->MaxMarketOrderVolume);
    ifItem.insert("minMarket", pif->MinMarketOrderVolume);

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    QString id = ifItem.value("symbol").toString();
    int row = instruments_row_.value(id);
    for (int i = 0; i < instruments_col_.count(); i++) {
        QVariant raw_val = ifItem.value(instruments_col_.at(i));
        QString str_val = raw_val.toString();
        if (raw_val.type() == QMetaType::Double || raw_val.type() == QMetaType::Float) {
            str_val = QString().sprintf("%6.1f", raw_val.toDouble());
        }

        QTableWidgetItem* item = new QTableWidgetItem(str_val);
        ui->tableWidget->setItem(row, i, item);
    }
}

void ContractForm::onTradeClosed()
{
    instruments_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(0);
}
