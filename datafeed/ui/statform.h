#ifndef STATFORM_H
#define STATFORM_H

#include <QMap>
#include <QWidget>

namespace Ui {
class StatForm;
}

class StatForm : public QWidget {
    Q_OBJECT

public:
    explicit StatForm(QWidget* parent = 0);
    ~StatForm();
    void init();
    void shutdown();

private slots:
    void on_pushButtonRefresh_clicked();
    void on_pushButtonShowData_clicked();
    void on_tableWidget_cellClicked(int row, int column);

private:
    void refresh();
    void statTick(QString symbol, QString exchange, QString name);
    void statBar(QString symbol, QString exchange, QString name);
    void onGotData(QString symbol, QString exchange, QString name, QString period, QString startDate, QString startTime, QString endDate, QString endTime);

private:
    Ui::StatForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;
};

#endif // STATFORM_H
