#ifndef ROBOTFORM_H
#define ROBOTFORM_H

#include <QMap>
#include <QWidget>

namespace Ui {
class RobotForm;
}

class RobotForm : public QWidget {
    Q_OBJECT

public:
    explicit RobotForm(QWidget* parent = 0);
    ~RobotForm();
    void init();
    void shutdown();

private:
    Ui::RobotForm* ui;

    QStringList table_col_;
    QMap<QString, int> table_row_;
};

#endif // ROBOTFORM_H
