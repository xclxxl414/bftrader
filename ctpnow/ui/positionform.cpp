#include "positionform.h"
#include "ctp_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_positionform.h"

PositionForm::PositionForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PositionForm)
{
    ui->setupUi(this);

    //设置列=
    table_col_ << "symbol"
               << "exchange"

               << "direction"
               << "position"
               << "price"
               << "frozen"
               << "ydPosition";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
    }

    // 调整参数=
    bfAdjustTableWidget(ui->tableWidget);
}

PositionForm::~PositionForm()
{
    delete ui;
}

void PositionForm::init()
{
    // ctpmgr
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotPosition, this, &PositionForm::onGotPosition);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotInstruments, this, &PositionForm::onGotInstruments);
}

void PositionForm::shutdown()
{
}

void PositionForm::onGotInstruments(QStringList ids, QStringList idsAll)
{
    ids_ = ids;
}

void PositionForm::onGotPosition(const BfPositionData& newPos)
{
    QString newSymbol = QString::fromStdString(newPos.symbol());
    QString newExchange = QString::fromStdString(newPos.exchange()); //ctp里面没有提供=
    void* newContract = g_sm->ctpMgr()->getContract(newSymbol);
    if (newContract) {
        newExchange = CtpUtils::getExchangeFromContract(newContract);
    }

    QString newKey = QString().sprintf("%s.%s.%d", newSymbol.toStdString().c_str(), newExchange.toStdString().c_str(), newPos.direction());
    if (newPos.position() == 0 && newPos.frozen() == 0) {
        if (positions_.contains(newKey)) {
            positions_.remove(newKey);
        }
    } else {
        positions_[newKey] = newPos;
    }

    // 更新界面=
    table_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(positions_.size());
    QStringList keys = positions_.keys();
    keys.sort();
    for (int i = 0; i < keys.length(); i++) {
        QString id = keys.at(i);
        table_row_[id] = i;
    }

    for (auto pos : positions_) {
        QString symbol = QString::fromStdString(pos.symbol());
        QString exchange = QString::fromStdString(pos.exchange()); //ctp里面没有提供=
        int volumeMultiple = 1;
        void* contract = g_sm->ctpMgr()->getContract(symbol);
        if (contract) {
            exchange = CtpUtils::getExchangeFromContract(contract);
            volumeMultiple = CtpUtils::getVolumeMultipleFromContract(contract);
        }

        QVariantMap vItem;
        vItem.insert("symbol", symbol);
        vItem.insert("exchange", exchange);

        vItem.insert("direction", CtpUtils::formatDirection(pos.direction()));
        vItem.insert("position", pos.position());
        vItem.insert("price", pos.price() / volumeMultiple);
        vItem.insert("frozen", pos.frozen());
        vItem.insert("ydPosition", pos.ydposition());

        //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
        QString key = QString().sprintf("%s.%s.%d", symbol.toStdString().c_str(), exchange.toStdString().c_str(), pos.direction());
        int row = table_row_.value(key);
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
}

void PositionForm::on_pushButtonQueryPosition_clicked()
{
    QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryPosition", Qt::QueuedConnection);
}

void PositionForm::on_pushButtonCloseAll_clicked()
{
    for (auto pos : positions_) {
        // 没有持仓的pass
        if (pos.position() == 0) {
            continue;
        }

        QString symbol = pos.symbol().c_str();
        QString exchange = pos.exchange().c_str();
        if (exchange.length() == 0) {
            void* contract = g_sm->ctpMgr()->getContract(symbol);
            exchange = CtpUtils::getExchangeFromContract(contract);
        }

        // 没有订阅的pass
        if (!ids_.contains(symbol)) {
            BfInfo(symbol + " not subscrible,please close byhand");
            continue;
        }

        void* tick = g_sm->ctpMgr()->getLatestTick(symbol);
        BfTickData bfTick;
        CtpUtils::translateTick(tick, nullptr, bfTick);

        // 限价单=
        BfOffset offset = OFFSET_CLOSE;
        BfPriceType priceType = PRICETYPE_LIMITPRICE;
        //todo(hege):这里要判断是否超过了contract->maxLimit
        //todo(hege):这里要判断昨持仓和今持仓，先平昨再平今=
        int volume = pos.position();

        //空单-->最高价+多+平=
        //多单-->最低价+空+平=
        double price = bfTick.upperlimit();
        BfDirection direction = DIRECTION_LONG;
        if (pos.direction() == DIRECTION_NET || pos.direction() == DIRECTION_LONG) {
            direction = DIRECTION_SHORT;
            price = bfTick.lowerlimit();
        }

        BfSendOrderReq req;
        req.set_symbol(symbol.toStdString());
        req.set_exchange(exchange.toStdString());
        req.set_price(price);
        req.set_volume(volume);
        req.set_direction(direction);
        req.set_offset(offset);
        req.set_pricetype(priceType);

        QMetaObject::invokeMethod(g_sm->ctpMgr(), "sendOrder", Qt::QueuedConnection, Q_ARG(BfSendOrderReq, req));
    }
}
