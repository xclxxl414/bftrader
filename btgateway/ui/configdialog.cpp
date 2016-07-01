#include "configdialog.h"
#include "profile.h"
#include "servicemgr.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    setWindowTitle("config");
    setWindowIcon(QIcon(":/images/gateway.png"));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::save()
{
    Profile* profile = g_sm->profile();
    profile->put("symbol", ui->symbol->text());
    profile->put("exchange", ui->exchange->text());
    profile->put("fromDate", ui->fromDate->text());
    profile->put("fromTime", ui->fromTime->text());
    profile->put("toDate", ui->toDate->text());
    profile->put("toTime", ui->toTime->text());
    profile->commit();
}

void ConfigDialog::load()
{
    Profile* profile = g_sm->profile();
    ui->symbol->setText(profile->get("symbol", "SR609").toString());
    ui->exchange->setText(profile->get("exchange", "CZCE").toString());
    ui->fromDate->setText(profile->get("fromDate", "20160626").toString());
    ui->fromTime->setText(profile->get("fromTime", "09:00:00.000").toString());
    ui->toDate->setText(profile->get("toDate", "20160629").toString());
    ui->toTime->setText(profile->get("toTime", "23:30:00.000").toString());
}
