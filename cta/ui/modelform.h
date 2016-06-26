#ifndef MODELFORM_H
#define MODELFORM_H

#include <QMap>
#include <QWidget>

namespace Ui {
class ModelForm;
}

class ModelForm : public QWidget {
    Q_OBJECT

public:
    explicit ModelForm(QWidget* parent = 0);
    ~ModelForm();
    void init();
    void shutdown();

private:
    Ui::ModelForm* ui;

    QStringList table_col_;
    QMap<QString, int> table_row_;
};

#endif // MODELFORM_H
