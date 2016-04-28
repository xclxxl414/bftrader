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
               << "ydPosition"
               << "price"
               << "frozen"

               << "key";
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
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotContracts, this, &PositionForm::onGotContracts);
}

void PositionForm::shutdown()
{
}

void PositionForm::onGotContracts(QStringList ids, QStringList idsAll)
{
    ids_ = ids;
}

void PositionForm::onGotPosition(const BfPositionData& newPos)
{
    QString newSymbol = QString::fromStdString(newPos.symbol());
    QString newExchange = QString::fromStdString(newPos.exchange()); //ctp里面没有提供=
    void* newContract = g_sm->ctpMgr()->getContract(newSymbol);
    newExchange = CtpUtils::getExchangeFromContract(newContract);

    QString newKey = QString().sprintf("%s.%s.%d", newSymbol.toStdString().c_str(), newExchange.toStdString().c_str(), newPos.direction());

    // 需要累加=
    BfPositionData combPos;
    if (!positions_.contains(newKey)){
        combPos = newPos;
    }else{
        BfPositionData oldPos = positions_[newKey];
        combPos = oldPos;
        combPos.set_position(oldPos.position() + newPos.position());
        combPos.set_ydposition(oldPos.ydposition() + newPos.ydposition());
        combPos.set_frozen(oldPos.frozen() + newPos.frozen());
        if(combPos.position()>0){
            combPos.set_price( (oldPos.price()*oldPos.position()+newPos.price()*newPos.position())/combPos.position());
        }
    }

    // 滤掉空的=
    if (combPos.position()==0 && combPos.frozen()==0){
        if(positions_.contains(newKey)){
            positions_.remove(newKey);
        }
    }else{
        positions_[newKey] = combPos;
    }

    // 更新界面=
    table_row_.clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(positions_.size());
    QStringList keys = positions_.keys();
    keys.sort();
    for (int i = 0; i < keys.length(); i++) {
        QString key = keys.at(i);
        table_row_[key] = i;

        QTableWidgetItem* item = new QTableWidgetItem(key);
        ui->tableWidget->setItem(i, table_col_.indexOf("key"), item);
    }

    for (auto pos : positions_) {
        QString symbol = QString::fromStdString(pos.symbol());
        QString exchange = QString::fromStdString(pos.exchange()); //ctp里面没有提供=
        int volumeMultiple = 1;
        void* contract = g_sm->ctpMgr()->getContract(symbol);
        exchange = CtpUtils::getExchangeFromContract(contract);
        volumeMultiple = CtpUtils::getVolumeMultipleFromContract(contract);

        QVariantMap vItem;
        vItem.insert("symbol", symbol);
        vItem.insert("exchange", exchange);

        vItem.insert("direction", CtpUtils::formatDirection(pos.direction()));
        vItem.insert("position", pos.position());
        vItem.insert("ydPosition", pos.ydposition());
        vItem.insert("price", pos.price() / volumeMultiple);
        vItem.insert("frozen", pos.frozen());

        QString key = QString().sprintf("%s.%s.%d", symbol.toStdString().c_str(), exchange.toStdString().c_str(), pos.direction());
        vItem.insert("key", key);

        //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
        int row = table_row_.value(key);
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
    table_row_.clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(positions_.size());
    positions_.clear();

    QMetaObject::invokeMethod(g_sm->ctpMgr(), "queryPosition", Qt::QueuedConnection);
}

// 如果不是上期所，平今仓可用close或closeToday，平昨仓可用close或closeYesterday=
// 如果是上期所，平今仓只可用closeToday，平昨仓可用close或closeYesterday=
// 那成交回报的流水回来的时侯:
// CloseYesterday=> insert =>CloseYesterday
// CloseToday=> insert =>CloseToday
// 也存在:closeToday =>close或closeYesterday =>close的情况=
// 参考：http://www.cnblogs.com/xiaouisme/p/4654750.html
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

        // 取不到tick的pass，比如晚上if不开盘=
        void* tick = g_sm->ctpMgr()->getLatestTick(symbol);
        if (!tick) {
            BfInfo(symbol + " has no tick,please close byhand");
            continue;
        }
        BfTickData bfTick;
        CtpUtils::translateTick(tick, nullptr, &bfTick);

        // 平昨=
        if (pos.ydposition() > 0) {
            // 限价单=
            BfOffset offset = OFFSET_CLOSE;
            BfPriceType priceType = PRICETYPE_LIMITPRICE;
            //todo(hege):这里要判断是否超过了contract->maxLimit
            //todo(hege):这里要判断昨持仓和今持仓，先平昨再平今=
            int volume = pos.ydposition();

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

        // 平今=
        if (pos.position() - pos.ydposition() > 0) {
            // 限价单=
            BfOffset offset = OFFSET_CLOSETODAY;
            BfPriceType priceType = PRICETYPE_LIMITPRICE;
            //todo(hege):这里要判断是否超过了contract->maxLimit
            int volume = pos.position() - pos.ydposition();

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
}
