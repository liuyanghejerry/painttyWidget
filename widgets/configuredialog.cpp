#include "configuredialog.h"
#include "ui_configuredialog.h"

ConfigureDialog::ConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}
