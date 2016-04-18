#ifndef ACCOUNTFORM_H
#define ACCOUNTFORM_H

#include <QWidget>

namespace Ui {
class AccountForm;
}

class AccountForm : public QWidget
{
    Q_OBJECT

public:
    explicit AccountForm(QWidget *parent = 0);
    ~AccountForm();

private:
    Ui::AccountForm *ui;
};

#endif // ACCOUNTFORM_H
