#ifndef FINISHEDORDERFORM_H
#define FINISHEDORDERFORM_H

#include <QWidget>

namespace Ui {
class FinishedOrderForm;
}

class FinishedOrderForm : public QWidget
{
    Q_OBJECT

public:
    explicit FinishedOrderForm(QWidget *parent = 0);
    ~FinishedOrderForm();
    void init();
    void shutdown();

private:
    Ui::FinishedOrderForm *ui;
};

#endif // FINISHEDORDERFORM_H
