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
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::save()
{
    Profile* profile = g_sm->profile();
    profile->put("userId", ui->userId->text());
    profile->put("brokerId", ui->brokerId->text());
    profile->put("frontMd", ui->frontMd->text());
    profile->put("frontTd", ui->frontTd->text());
    profile->put("idPrefixList", ui->idPrefixList->text());
    profile->commit();
}

void ConfigDialog::load()
{
    Profile* profile = g_sm->profile();
    ui->userId->setText(profile->get("userId", "").toString());
    ui->brokerId->setText(profile->get("brokerId", "").toString());
    ui->frontMd->setText(profile->get("frontMd", "").toString());
    ui->frontTd->setText(profile->get("frontTd", "").toString());
    ui->idPrefixList->setText(profile->get("idPrefixList", "if;ih;ic;sr;rb;pp").toString());
}
