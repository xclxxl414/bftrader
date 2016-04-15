#ifndef TRADEFORM_H
#define TRADEFORM_H

#include <QWidget>

namespace Ui {
class TradeForm;
}

class TradeForm : public QWidget
{
    Q_OBJECT

public:
    explicit TradeForm(QWidget *parent = 0);
    ~TradeForm();
    void init();
    void shutdown();

private:
    Ui::TradeForm *ui;
};

#endif // TRADEFORM_H
