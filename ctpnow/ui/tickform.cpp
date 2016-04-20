#include "tickform.h"
#include "ThostFtdcUserApiStruct.h"
#include "ctpmgr.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
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

                     << "bidPrice"
                     << "askPrice"
                     << "bidVolume"
                     << "askVolume"

                     << "volume"
                     << "openInterest"
                     << "lastVolume"

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
    // tablewidget更新ui太慢了(是自适应高度和宽度搞的，去掉自适应后好了)，不过改成500毫秒的定时器也不错=
    //QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotTick, this, &TickForm::onGotTick);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotInstruments, this, &TickForm::onGotInstruments);
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
    auto curMdf = (CThostFtdcDepthMarketDataField*)curTick;
    auto preMdf = (CThostFtdcDepthMarketDataField*)preTick;

    QVariantMap mdItem;
    mdItem.insert("symbol", curMdf->InstrumentID);
    // tick里面的exchange不一定有=
    QString exchange = curMdf->ExchangeID;
    if (exchange.trimmed().length() == 0) {
        auto contract = (CThostFtdcInstrumentField*)g_sm->ctpMgr()->getContract(curMdf->InstrumentID);
        if (contract) {
            exchange = contract->ExchangeID;
        }
    }
    mdItem.insert("exchange", exchange);
    mdItem.insert("date", curMdf->ActionDay);
    mdItem.insert("time", QString().sprintf("%s.%3d", curMdf->UpdateTime, curMdf->UpdateMillisec));
    mdItem.insert("lastPrice", curMdf->LastPrice);
    mdItem.insert("volume", curMdf->Volume);
    mdItem.insert("openInterest", curMdf->OpenInterest);
    mdItem.insert("lastVolume", preMdf ? curMdf->Volume - preMdf->Volume : 1);

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

    this->updateTickTimer_->start();
}

void TickForm::onTradeWillBegin()
{
    this->updateTickTimer_->stop();

    instruments_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(0);
}

void TickForm::onUpdateTick()
{
    for (int i = 0; i < instruments_row_.size(); i++) {
        QString id = instruments_row_.key(i);
        void* curTick = g_sm->ctpMgr()->getLatestTick(id);
        void* preTick = g_sm->ctpMgr()->getPreLatestTick(id);
        if (curTick) {
            onGotTick(curTick, preTick);
        }
    }
}
