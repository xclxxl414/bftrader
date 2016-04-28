#ifndef HISTORYTICKFORM_H
#define HISTORYTICKFORM_H

#include "ctpmgr.h"
#include <QVector>
#include <QWidget>

namespace Ui {
class HistoryTickForm;
}

class HistoryTickForm : public QWidget {
    Q_OBJECT

public:
    explicit HistoryTickForm(QWidget* parent = 0);
    ~HistoryTickForm();
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
    Ui::HistoryTickForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;

    QString symbol_;
    QString exchange_;
    QVector<double> x_;
    QVector<double> y_;
};

#endif // HISTORYTICKFORM_H
