#ifndef PENDINGORDERFORM_H
#define PENDINGORDERFORM_H

#include <QWidget>

namespace Ui {
class PendingOrderForm;
}

class PendingOrderForm : public QWidget {
    Q_OBJECT

public:
    explicit PendingOrderForm(QWidget* parent = 0);
    ~PendingOrderForm();
    void init();
    void shutdown();

private:
    Ui::PendingOrderForm* ui;
};

#endif // PENDINGORDERFORM_H
