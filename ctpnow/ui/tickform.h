#ifndef TICKFORM_H
#define TICKFORM_H

#include <QWidget>
#include <QMap>

namespace Ui {
class TickForm;
}

class TickForm : public QWidget
{
    Q_OBJECT

public:
    explicit TickForm(QWidget *parent = 0);
    ~TickForm();
    void init();
    void shutdown();

public slots:
    void onGotInstruments(QStringList ids);
    void onGotTick(void* curTick,void* preTick);
    void onTradeClosed();

private:
    Ui::TickForm *ui;

    QMap<QString,int> instruments_row_;
    QStringList instruments_col_;
};

#endif // TICKFORM_H
