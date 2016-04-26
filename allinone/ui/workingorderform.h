#ifndef WORKINGORDERFORM_H
#define WORKINGORDERFORM_H

#include <QMap>
#include <QWidget>

#include "ctpmgr.h"

namespace Ui {
class WorkingOrderForm;
}

class WorkingOrderForm : public QWidget {
    Q_OBJECT

public:
    explicit WorkingOrderForm(QWidget* parent = 0);
    ~WorkingOrderForm();
    void init();
    void shutdown();

private slots:
    void onGotOrder(const BfOrderData& data);
    void on_pushButtonQueryOrders_clicked();
    void on_pushButtonCancelAll_clicked();

private:
    Ui::WorkingOrderForm* ui;
    QStringList table_col_;
    QMap<QString, int> table_row_;

    QMap<QString, BfOrderData> orders_;
};

#endif // WORKINGORDERFORM_H
