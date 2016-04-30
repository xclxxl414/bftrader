#ifndef TICKFORM_H
#define TICKFORM_H

#include "gatewaymgr.h"
#include <QMap>
#include <QVector>
#include <QWidget>

namespace Ui {
class TickForm;
}

class TickForm : public QWidget {
    Q_OBJECT

public:
    explicit TickForm(QWidget* parent = 0);
    ~TickForm();
    void init(QString symbol, QString exchange);

private slots:
    void on_first128_clicked();
    void on_next128_clicked();
    void on_pre128_clicked();
    void on_last128_clicked();
    void on_seekButton_clicked();
    void on_delButton_clicked();
    void on_tableWidget_cellClicked(int row, int column);

private:
    void onGotTick(QString key, const BfTickData& bfItem);
    void initGraph();
    void drawGraph();

private:
    Ui::TickForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;

    QString symbol_;
    QString exchange_;
    QVector<double> x_;
    QVector<double> y_;
};

#endif // TickForm_H
