#ifndef POSITIONFORM_H
#define POSITIONFORM_H

#include "ctpmgr.h"
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
    void onGotContracts(QStringList ids, QStringList idsAll);
    void onGotPosition(const BfPositionData& pos);
    void on_pushButtonQueryPosition_clicked();
    void on_pushButtonCloseAll_clicked();

private:
    Ui::PositionForm* ui;
    QStringList table_col_;
    QMap<QString, int> table_row_;

    QMap<QString, BfPositionData> positions_;
    QStringList ids_;
};

#endif // POSITIONFORM_H
