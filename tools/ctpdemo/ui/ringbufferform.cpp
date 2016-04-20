#include "ringbufferform.h"
#include "ThostFtdcUserApiStruct.h"
#include "ctpmgr.h"
#include "datapump.h"
#include "dbservice.h"
#include "mdsm.h"
#include "ringbuffer.h"
#include "servicemgr.h"
#include "ui_ringbufferform.h"

RingBufferForm::RingBufferForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RingBufferForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "TradingDay"
               << "UpdateTime"
               << "UpdateMillisec"
               << "LastPrice"
               << "Volume"
               << "OpenInterest"
               << "BidPrice1"
               << "BidVolume1"
               << "AskPrice1"
               << "AskVolume1";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }
}

RingBufferForm::~RingBufferForm()
{
    delete ui;
}

void RingBufferForm::init(QString id)
{
    id_ = id;
    this->setWindowTitle(QString("ringbuffer-") + id);
    scanTicks();
}

void RingBufferForm::scanTicks()
{
    DataPump* datapump = g_sm->dataPump();
    if (!datapump) {
        return;
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    RingBuffer* rb = datapump->getRingBuffer(id_);

    int head = rb->head();
    if (head < 0) {
        return;
    }

    for (int i = 0; i < rb->count() / 2; i++) {
        void* p = rb->get(head);
        if (p == nullptr) {
            return;
        }

        onGotTick(p);
        head -= 1;
        if (head == -1) {
            head += rb->count();
        }
    }
}

void RingBufferForm::onGotTick(void* tick)
{
    auto mdf = (CThostFtdcDepthMarketDataField*)tick;

    QVariantMap mdItem;
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

void RingBufferForm::on_refreshButton_clicked()
{
    scanTicks();
}
