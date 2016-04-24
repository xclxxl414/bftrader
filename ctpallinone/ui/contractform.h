#ifndef CONTRACTFORM_H
#define CONTRACTFORM_H

#include <QMap>
#include <QWidget>

namespace Ui {
class ContractForm;
}

class ContractForm : public QWidget {
    Q_OBJECT

public:
    explicit ContractForm(QWidget* parent = 0);
    ~ContractForm();
    void init();
    void shutdown();

private slots:
    void onGotContracts(QStringList ids, QStringList idsAll);
    void onTradeWillBegin();

private:
    void onGotContract(void* contract);

private:
    Ui::ContractForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;
};

#endif // CONTRACTFORM_H
