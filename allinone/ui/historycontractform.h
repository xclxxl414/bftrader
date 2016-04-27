#ifndef HISTORYCONTRACTFORM_H
#define HISTORYCONTRACTFORM_H

#include <QMap>
#include <QStringList>
#include <QWidget>

#include "ctpmgr.h"

namespace Ui {
class HistoryContractForm;
}

class HistoryContractForm : public QWidget {
    Q_OBJECT

public:
    explicit HistoryContractForm(QWidget* parent = 0);
    ~HistoryContractForm();

public:
    void init();

private slots:
    void on_refreshButton_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);
    void on_tableWidget_cellClicked(int row, int column);
    void on_pushButtonTick_clicked();

private:
    void refresh();
    void onGotContract(QString key, const BfContractData& bfContract);

private:
    Ui::HistoryContractForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;
};

#endif // HISTORYCONTRACTFORM_H
