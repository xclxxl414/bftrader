#include "ringbufferform.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "historytickform.h"
#include "ringbuffer.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_ringbufferform.h"

RingBufferForm::RingBufferForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RingBufferForm)
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
               << "lowerLimit";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

RingBufferForm::~RingBufferForm()
{
    delete ui;
}

void RingBufferForm::init(QString symbol, QString exchange)
{
    symbol_ = symbol;
    exchange_ = exchange;
    this->setWindowTitle(QString("ringbuffer-tick-") + symbol_);
    scanTicks();

    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &RingBufferForm::onTradeWillBegin);
}

void RingBufferForm::scanTicks()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    RingBuffer* rb = g_sm->ctpMgr()->getRingBuffer(symbol_);

    int head = rb->head();
    if (head < 0) {
        return;
    }

    for (int i = 0; i < rb->count() / 2; i++) {
        void* curTick = rb->get(head);
        if (curTick == nullptr) {
            return;
        }

        head -= 1;
        if (head == -1) {
            head += rb->count();
        }
        void* preTick = rb->get(head);

        onGotTick(curTick, preTick);
    }
}

void RingBufferForm::onGotTick(void* curTick, void* preTick)
{
    BfTickData bfTick;
    CtpUtils::translateTick(curTick, preTick, &bfTick);

    QVariantMap vItem;
    vItem.insert("symbol", bfTick.symbol().c_str());
    // tick里面的exchange不一定有=
    QString exchange = bfTick.exchange().c_str();
    if (exchange.trimmed().length() == 0) {
        void* contract = g_sm->ctpMgr()->getContract(bfTick.symbol().c_str());
        if (contract) {
            exchange = CtpUtils::getExchangeFromContract(contract);
        }
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

void RingBufferForm::on_historyButton_clicked()
{
    HistoryTickForm* form = new HistoryTickForm();
    form->setWindowFlags(Qt::Window);
    form->init(symbol_, exchange_);
    centerWindow(form);
    form->show();
}

void RingBufferForm::on_refreshButton_clicked()
{
    scanTicks();
}

void RingBufferForm::onTradeWillBegin()
{
    this->close();
}
