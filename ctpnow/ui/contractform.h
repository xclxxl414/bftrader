#ifndef CONTRACTFORM_H
#define CONTRACTFORM_H

#include <QWidget>
#include <QMap>

namespace Ui {
class ContractForm;
}

class ContractForm : public QWidget
{
    Q_OBJECT

public:
    explicit ContractForm(QWidget *parent = 0);
    ~ContractForm();
    void init();
    void shutdown();

public slots:
    void onGotInstruments(QStringList ids);
    void onGotTick(void* tick);

private:
    Ui::ContractForm *ui;

    QMap<QString,int> instruments_row_;
    QStringList instruments_col_;
};

#endif // CONTRACTFORM_H
