#ifndef DEBUGFORM_H
#define DEBUGFORM_H

#include <QWidget>

namespace Ui {
class DebugForm;
}

class DebugForm : public QWidget {
    Q_OBJECT

public:
    explicit DebugForm(QWidget* parent = 0);
    ~DebugForm();
    void init();
    void shutdown();

public slots:
    void onLog(QString when, QString msg);

private:
    Ui::DebugForm* ui;

    QStringList table_col_;
};

#endif // DEBUGFORM_H
