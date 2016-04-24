#ifndef ACCOUNTFORM_H
#define ACCOUNTFORM_H

#include "ctpmgr.h"
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
    void onGotAccount(const BfAccountData& account);
    void on_pushButtonQueryAccount_clicked();
    void on_pushButtonFeedback_clicked();

private:
    QString formatDouble(double val);

private:
    Ui::AccountForm* ui;
};

#endif // ACCOUNTFORM_H
