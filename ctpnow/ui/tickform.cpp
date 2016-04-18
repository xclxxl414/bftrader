#include "tickform.h"
#include "ThostFtdcUserApiStruct.h"
#include "ctpmgr.h"
#include "nofocusdelegate.h"
#include "servicemgr.h"
#include "ui_tickform.h"

TickForm::TickForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TickForm)
{
    ui->setupUi(this);

    //设置列=
    instruments_col_ << "symbol"
                     << "exchange"

                     << "date"
                     << "time"
                     << "lastPrice"
                     << "volume"
                     << "openInterest"
                     << "lastVolume"
                     << "lastOpenInterest"

                     << "bidPrice"
                     << "askPrice"
                     << "bidVolume"
                     << "askVolume"

                     << "openPrice"
                     << "highPrice"
                     << "lowPrice"
                     << "preClosePrice"
                     << "upperLimit"
                     << "lowerLimit";
    this->ui->tableWidget->setColumnCount(instruments_col_.length());
    for (int i = 0; i < instruments_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(instruments_col_.at(i)));
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
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTick, this, &TickForm::onGotTick);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotInstruments, this, &TickForm::onGotInstruments);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeClosed, this, &TickForm::onTradeClosed);
}

void TickForm::shutdown()
{
}

void TickForm::onGotTick(void* curTick, void* preTick)
{
    auto curMdf = (CThostFtdcDepthMarketDataField*)curTick;
    auto preMdf = (CThostFtdcDepthMarketDataField*)preTick;

    QVariantMap mdItem;
    mdItem.insert("symbol", curMdf->InstrumentID);
    mdItem.insert("exchange", curMdf->ExchangeID);

    mdItem.insert("date", curMdf->ActionDay);
    mdItem.insert("time", QString().sprintf("%s.%3d", curMdf->UpdateTime, curMdf->UpdateMillisec));
    mdItem.insert("lastPrice", curMdf->LastPrice);
    mdItem.insert("volume", curMdf->Volume);
    mdItem.insert("openInterest", curMdf->OpenInterest);
    mdItem.insert("lastVolume", preMdf ? curMdf->Volume - preMdf->Volume : 1);
    mdItem.insert("lastOpenInterest", preMdf ? curMdf->OpenInterest - preMdf->OpenInterest : 1.0);

    mdItem.insert("bidPrice", curMdf->BidPrice1);
    mdItem.insert("askPrice", curMdf->AskPrice1);
    mdItem.insert("bidVolume", curMdf->BidVolume1);
    mdItem.insert("askVolume", curMdf->AskVolume1);

    mdItem.insert("openPrice", curMdf->OpenPrice);
    mdItem.insert("highPrice", curMdf->HighestPrice);
    mdItem.insert("lowPrice", curMdf->LowestPrice);
    mdItem.insert("preClosePrice", curMdf->PreClosePrice);
    mdItem.insert("upperLimit", curMdf->UpperLimitPrice);
    mdItem.insert("lowerLimit", curMdf->LowerLimitPrice);

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    QString id = mdItem.value("symbol").toString();
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

void TickForm::onTradeClosed()
{
    instruments_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(0);
}

void TickForm::on_pushButtonFit_clicked()
{
    bfFitTableWidget(ui->tableWidget);
}
