#ifndef WORKINGORDERFORM_H
#define WORKINGORDERFORM_H

#include <QMap>
#include <QWidget>

#include "gatewaymgr.h"

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
    void onGotOrder(QString gatewayId, const BfOrderData& data);
    void onGotNotification(QString gatewayId, const BfNotificationData& note);
    void on_pushButtonQueryOrders_clicked();
    void on_pushButtonCancelAll_clicked();

private:
    void updateUI();

private:
    Ui::WorkingOrderForm* ui;
    QStringList table_col_;
    QMap<QString, int> table_row_;

    QMap<QString, BfOrderData> orders_;
    bool querying_ = false;

    QString gatewayId_;
};

#endif // WORKINGORDERFORM_H
