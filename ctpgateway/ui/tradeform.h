#ifndef TRADEFORM_H
#define TRADEFORM_H

#include "ctpmgr.h"
#include <QWidget>

namespace Ui {
class TradeForm;
}

class TradeForm : public QWidget {
    Q_OBJECT

public:
    explicit TradeForm(QWidget* parent = 0);
    ~TradeForm();
    void init();
    void shutdown();

private slots:
    void onGotTrade(const BfTradeData& trade);

private:
    Ui::TradeForm* ui;
    QStringList table_col_;
};

#endif // TRADEFORM_H
