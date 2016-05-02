#ifndef POSITIONFORM_H
#define POSITIONFORM_H

#include <QWidget>
#include <QMap>

namespace Ui {
class PositionForm;
}

class PositionForm : public QWidget
{
    Q_OBJECT

public:
    explicit PositionForm(QWidget *parent = 0);
    ~PositionForm();
    void init();
    void shutdown();

private:
    Ui::PositionForm *ui;

    QStringList table_col_;
    QMap<QString, int> table_row_;
};

#endif // POSITIONFORM_H
