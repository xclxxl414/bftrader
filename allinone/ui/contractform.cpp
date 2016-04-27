#include "contractform.h"
#include "ctp_utils.h"
#include "ctpmgr.h"
#include "encode_utils.h"
#include "servicemgr.h"
#include "tablewidget_helper.h"
#include "ui_contractform.h"

ContractForm::ContractForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ContractForm)
{
    ui->setupUi(this);

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
               << "minMarket";
    this->ui->tableWidget->setColumnCount(table_col_.length());
    for (int i = 0; i < table_col_.length(); i++) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(table_col_.at(i)));
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
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::gotContracts, this, &ContractForm::onGotContracts);
    QObject::connect(g_sm->ctpMgr(), &CtpMgr::tradeWillBegin, this, &ContractForm::onTradeWillBegin);
}

void ContractForm::shutdown()
{
}

void ContractForm::onGotContracts(QStringList ids, QStringList idsAll)
{
    //设置行，按排序后合约来，一个合约一行=
    table_row_.clear();
    //QStringList sorted_ids = idsAll;
    QStringList sorted_ids = ids;
    sorted_ids.sort();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(sorted_ids.length());
    for (int i = 0; i < sorted_ids.length(); i++) {
        QString id = sorted_ids.at(i);
        table_row_[id] = i;
        QTableWidgetItem* item = new QTableWidgetItem(id);
        ui->tableWidget->setItem(i, 0, item);
    }

    //设置行内容=
    for (int i = 0; i < sorted_ids.length(); i++) {
        QString id = sorted_ids.at(i);
        void* contract = g_sm->ctpMgr()->getContract(id);
        onGotContract(contract);
    }
}

void ContractForm::onGotContract(void* contract)
{
    BfContractData bfItem;
    CtpUtils::translateContract(contract, &bfItem);

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

    //根据id找到对应的行，然后用列的text来在map里面取值设置到item里面=
    QString id = vItem.value("symbol").toString();
    int row = table_row_.value(id);
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

void ContractForm::onTradeWillBegin()
{
    table_row_.clear();
    this->ui->tableWidget->clearContents();
    this->ui->tableWidget->setRowCount(0);
}
