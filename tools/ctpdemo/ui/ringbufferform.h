#ifndef RINGBUFFERFORM_H
#define RINGBUFFERFORM_H

#include <QWidget>

namespace Ui {
class RingBufferForm;
}

class RingBufferForm : public QWidget {
    Q_OBJECT

public:
    explicit RingBufferForm(QWidget* parent = 0);
    ~RingBufferForm();
    void init(QString id);

private:
    void scanTicks();
    void onGotTick(void* tick);

private slots:
    void on_refreshButton_clicked();

private:
    Ui::RingBufferForm* ui;
    QStringList table_col_;
    QString id_;
};

#endif // RINGBUFFERFORM_H
