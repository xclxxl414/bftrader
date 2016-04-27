#ifndef CTAFORM_H
#define CTAFORM_H

#include <QWidget>
#include <QMap>

namespace Ui {
class CtaForm;
}

class CtaForm : public QWidget
{
    Q_OBJECT

public:
    explicit CtaForm(QWidget *parent = 0);
    ~CtaForm();
    void init();
    void shutdown();

private:
    Ui::CtaForm *ui;

    QStringList table_col_;
    QMap<QString, int> table_row_;
};

#endif // CTAFORM_H
