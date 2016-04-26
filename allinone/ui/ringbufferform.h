#ifndef RINGBUFFERFORM_H
#define RINGBUFFERFORM_H

#include <QMap>
#include <QWidget>

namespace Ui {
class RingBufferForm;
}

// todo(hege): onTradeWillBegin,Close it!!!
class RingBufferForm : public QWidget {
    Q_OBJECT

public:
    explicit RingBufferForm(QWidget* parent = 0);
    ~RingBufferForm();
    void init(QString symbol, QString exchange);

private:
    void scanTicks();
    void onGotTick(void* curTick, void* preTick);

private slots:
    void onTradeWillBegin();
    void on_historyButton_clicked();
    void on_refreshButton_clicked();

private:
    Ui::RingBufferForm* ui;

    QMap<QString, int> table_row_;
    QStringList table_col_;

    QString symbol_;
    QString exchange_;
};

#endif // RINGBUFFERFORM_H
