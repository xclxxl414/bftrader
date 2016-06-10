#ifndef ERRORFORM_H
#define ERRORFORM_H

#include <QWidget>

namespace Ui {
class ErrorForm;
}

class ErrorForm : public QWidget {
    Q_OBJECT

public:
    explicit ErrorForm(QWidget* parent = 0);
    ~ErrorForm();
    void init();
    void shutdown();

public slots:
    void onLog(QString when, QString msg);

private:
    Ui::ErrorForm* ui;

    QStringList table_col_;
};

#endif // ERRORFORM_H
