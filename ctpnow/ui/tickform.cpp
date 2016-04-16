#include "tickform.h"
#include "ui_tickform.h"
#include "servicemgr.h"
#include "ctpmgr.h"
#include "ThostFtdcUserApiStruct.h"

TickForm::TickForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TickForm)
{
    ui->setupUi(this);

    //设置列=
    instruments_col_  << "InstrumentID" << "TradingDay" << "UpdateTime" << "UpdateMillisec" <<
        "LastPrice" << "Volume" << "OpenInterest" <<
        "BidPrice1" << "BidVolume1" << "AskPrice1" << "AskVolume1";
    this->ui->tableWidget->setColumnCount(instruments_col_.length());
    for (int i = 0; i < instruments_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(instruments_col_.at(i)));
    }
}

TickForm::~TickForm()
{
    delete ui;
}

void TickForm::init(){
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTick, this, &TickForm::onGotTick);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotInstruments, this, &TickForm::onGotInstruments);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeClosed, this, &TickForm::onTradeClosed);
}

void TickForm::shutdown(){

}

void TickForm::onGotTick(void* tick)
{
    auto mdf = (CThostFtdcDepthMarketDataField*)tick;

    QVariantMap mdItem;
    mdItem.insert("InstrumentID", mdf->InstrumentID);
    mdItem.insert("TradingDay", mdf->TradingDay);
    mdItem.insert("UpdateTime", mdf->UpdateTime);
    mdItem.insert("UpdateMillisec", mdf->UpdateMillisec);
    mdItem.insert("LastPrice", mdf->LastPrice);
    mdItem.insert("Volume", mdf->Volume);
    mdItem.insert("OpenInterest", mdf->OpenInterest);
    mdItem.insert("BidPrice1", mdf->BidPrice1);
    mdItem.insert("BidVolume1", mdf->BidVolume1);
    mdItem.insert("AskPrice1", mdf->AskPrice1);
    mdItem.insert("AskVolume1", mdf->AskVolume1);

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    QString id = mdItem.value("InstrumentID").toString();
    int row = instruments_row_.value(id);
    for (int i = 0; i < instruments_col_.count(); i++) {
        QVariant raw_val = mdItem.value(instruments_col_.at(i));
        QString str_val = raw_val.toString();
        if (raw_val.type() == QMetaType::Double || raw_val.type() == QMetaType::Float) {
            str_val = QString().sprintf("%6.1f", raw_val.toDouble());
        }

        QTableWidgetItem* item = new QTableWidgetItem(str_val);
        ui->tableWidget->setItem(row, i, item);
    }
}

void TickForm::onGotInstruments(QStringList ids)
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
}

void TickForm::onTradeClosed(){
    instruments_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(0);
}
