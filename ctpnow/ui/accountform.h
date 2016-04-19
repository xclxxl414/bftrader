#ifndef ACCOUNTFORM_H
#define ACCOUNTFORM_H

#include <QWidget>

namespace Ui {
class AccountForm;
}

class AccountForm : public QWidget {
    Q_OBJECT

public:
    explicit AccountForm(QWidget* parent = 0);
    ~AccountForm();
    void init();
    void shutdown();

private slots:
    void onGotAccount(double balance, double available, double margin, double closeProfit, double positionProfit);

    void on_pushButtonRefresh_clicked();

private:
    QString formatDouble(double val);

private:
    Ui::AccountForm* ui;
};

#endif // ACCOUNTFORM_H
