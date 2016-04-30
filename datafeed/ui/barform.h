#ifndef BARFORM_H
#define BARFORM_H

#include "gatewaymgr.h"
#include <QMap>
#include <QVector>
#include <QWidget>

namespace Ui {
class BarForm;
}

class BarForm : public QWidget {
    Q_OBJECT

public:
    explicit BarForm(QWidget* parent = 0);
    ~BarForm();
    void init(QString symbol, QString exchange, int period);

private slots:
    void on_first128_clicked();
    void on_next128_clicked();
    void on_pre128_clicked();
    void on_last128_clicked();
    void on_seekButton_clicked();
    void on_delButton_clicked();
    void on_tableWidget_cellClicked(int row, int column);

private:
    void onGotTick(QString key, const BfBarData& bfItem);
    void initGraph();
    void drawGraph();
    QString getPeriod();

private:
    Ui::BarForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;

    QString symbol_;
    QString exchange_;
    int period_ = 0;
    QVector<double> x_;
    QVector<double> y_;
};

#endif // BARFORM_H
