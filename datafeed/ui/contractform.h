#ifndef CONTRACTFORM_H
#define CONTRACTFORM_H

#include <QMap>
#include <QStringList>
#include <QWidget>

#include "gatewaymgr.h"

namespace Ui {
class ContractForm;
}

class ContractForm : public QWidget {
    Q_OBJECT

public:
    explicit ContractForm(QWidget* parent = 0);
    ~ContractForm();

public:
    void init();
    void shutdown();

private slots:
    void on_refreshButton_clicked();
    void on_tableWidget_cellClicked(int row, int column);
    void on_pushButtonTick_clicked();
    void on_pushButtonBar_clicked();

private:
    void refresh();
    void onGotContract(QString key, const BfContractData& bfContract);

private:
    Ui::ContractForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;
};

#endif // CONTRACTFORM_H
