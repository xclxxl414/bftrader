#ifndef TICKFORM_H
#define TICKFORM_H

#include <QMap>
#include <QTimer>
#include <QWidget>

namespace Ui {
class TickForm;
}

class TickForm : public QWidget {
    Q_OBJECT

public:
    explicit TickForm(QWidget* parent = 0);
    ~TickForm();
    void init();
    void shutdown();

public slots:
    void onGotInstruments(QStringList ids);
    void onGotTick(void* curTick, void* preTick);
    void onTradeClosed();

private slots:
    void on_pushButtonFit_clicked();
    void onUpdateTick();

private:
    Ui::TickForm* ui;
    QTimer* updateTickTimer_ = nullptr;

    QMap<QString, int> instruments_row_;
    QStringList instruments_col_;
};

#endif // TICKFORM_H
