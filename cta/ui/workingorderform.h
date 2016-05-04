#ifndef WORKINGORDERFORM_H
#define WORKINGORDERFORM_H

#include <QMap>
#include <QWidget>

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

private:
    Ui::WorkingOrderForm* ui;

    QStringList table_col_;
    QMap<QString, int> table_row_;
};

#endif // WORKINGORDERFORM_H
