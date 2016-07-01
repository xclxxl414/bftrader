#ifndef POSITIONFORM_H
#define POSITIONFORM_H

#include "gatewaymgr.h"
#include <QMap>
#include <QWidget>

namespace Ui {
class PositionForm;
}

class PositionForm : public QWidget {
    Q_OBJECT

public:
    explicit PositionForm(QWidget* parent = 0);
    ~PositionForm();
    void init();
    void shutdown();

private slots:
    void onGotContracts(QStringList symbolsMy, QStringList symbolsAll);
    void onGotPosition(const BfPositionData& pos);
    void onGotNotification(const BfNotificationData& note);
    void on_pushButtonQueryPosition_clicked();
    void on_pushButtonCloseAll_clicked();

private:
    void updateUI();

private:
    Ui::PositionForm* ui;
    QStringList table_col_;
    QMap<QString, int> table_row_;

    QMap<QString, BfPositionData> positions_;
    QStringList symbols_my_;
    bool querying_ = false;
};

#endif // POSITIONFORM_H
