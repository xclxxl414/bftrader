#ifndef PENDINGORDERFORM_H
#define PENDINGORDERFORM_H

#include <QMap>
#include <QWidget>

#include "ctpmgr.h"

namespace Ui {
class PendingOrderForm;
}

class PendingOrderForm : public QWidget {
    Q_OBJECT

public:
    explicit PendingOrderForm(QWidget* parent = 0);
    ~PendingOrderForm();
    void init();
    void shutdown();

private slots:
    void onGotOrder(const BfOrderData& data);
    void on_pushButtonCancelOrder_clicked();

private:
    Ui::PendingOrderForm* ui;
    QStringList table_col_;
    QMap<QString, int> table_row_;

    QMap<QString, BfOrderData> orders_;
};

#endif // PENDINGORDERFORM_H
