#ifndef FINISHEDORDERFORM_H
#define FINISHEDORDERFORM_H

#include <QWidget>

#include "ctpmgr.h"

namespace Ui {
class FinishedOrderForm;
}

class FinishedOrderForm : public QWidget {
    Q_OBJECT

public:
    explicit FinishedOrderForm(QWidget* parent = 0);
    ~FinishedOrderForm();
    void init();
    void shutdown();

private slots:
    void onGotOrder(const BfOrderData& data);

private:
    Ui::FinishedOrderForm* ui;
    QStringList table_col_;
    QMap<QString, int> table_row_;

    QMap<QString, BfOrderData> orders_;
};

#endif // FINISHEDORDERFORM_H
