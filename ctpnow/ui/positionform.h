#ifndef POSITIONFORM_H
#define POSITIONFORM_H

#include <QWidget>

namespace Ui {
class PositionForm;
}

class PositionForm : public QWidget {
    Q_OBJECT

public:
    explicit PositionForm(QWidget* parent = 0);
    ~PositionForm();
    void init();
    void shutdown();

private:
    Ui::PositionForm* ui;
    QStringList instruments_col_;
};

#endif // POSITIONFORM_H
