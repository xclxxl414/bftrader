#ifndef LOGFORM_H
#define LOGFORM_H

#include <QWidget>

namespace Ui {
class LogForm;
}

class LogForm : public QWidget {
    Q_OBJECT

public:
    explicit LogForm(QWidget* parent = 0);
    ~LogForm();
    void init();
    void shutdown();

public slots:
    void onInfo(QString when, QString msg);

private:
    Ui::LogForm* ui;

    QStringList table_col_;
};

#endif // LOGFORM_H
