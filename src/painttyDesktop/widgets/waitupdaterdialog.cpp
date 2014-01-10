#include "waitupdaterdialog.h"
#include "ui_waitupdaterdialog.h"

WaitUpdaterDialog::WaitUpdaterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaitUpdaterDialog)
{
    ui->setupUi(this);
}

WaitUpdaterDialog::~WaitUpdaterDialog()
{
    delete ui;
}
