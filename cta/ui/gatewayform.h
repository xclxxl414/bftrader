#ifndef GATEWAYFORM_H
#define GATEWAYFORM_H

#include <QMap>
#include <QWidget>

namespace Ui {
class GatewayForm;
}

class GatewayForm : public QWidget {
    Q_OBJECT

public:
    explicit GatewayForm(QWidget* parent = 0);
    ~GatewayForm();
    void init();
    void shutdown();

private slots:
    void on_pushButtonConnect_clicked();

    void on_pushButtonDisconnect_clicked();

private:
    Ui::GatewayForm* ui;

    QStringList table_col_;
    QMap<QString, int> table_row_;
};

#endif // GATEWAYFORM_H
