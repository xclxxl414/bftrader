#ifndef INSTRUMENTSFORM_H
#define INSTRUMENTSFORM_H

#include <QStringList>
#include <QWidget>

namespace Ui {
class InstrumentsForm;
}
namespace leveldb {
class DB;
}

class InstrumentsForm : public QWidget {
    Q_OBJECT

public:
    explicit InstrumentsForm(QWidget* parent = 0);
    ~InstrumentsForm();

public:
    void init(leveldb::DB* db);

private slots:
    void on_refreshButton_clicked();

private:
    void refresh();
    void onGotInstrument(void* p);

private:
    Ui::InstrumentsForm* ui;
    QStringList instruments_col_;
    leveldb::DB* db_ = nullptr;
};

#endif // INSTRUMENTSFORM_H
