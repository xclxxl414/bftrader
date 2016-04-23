#ifndef INFOFORM_H
#define INFOFORM_H

#include <QWidget>

namespace Ui {
class InfoForm;
}

class InfoForm : public QWidget {
    Q_OBJECT

public:
    explicit InfoForm(QWidget* parent = 0);
    ~InfoForm();
    void init();
    void shutdown();

public slots:
    void onLog(QString when, QString msg);

private:
    Ui::InfoForm* ui;

    QStringList table_col_;
};

#endif // INFOFORM_H
