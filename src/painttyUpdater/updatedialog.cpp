#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::print(const QString &content)
{
    auto text = ui->textEdit->toPlainText();
    ui->textEdit->setText(text+content);
}

void UpdateDialog::setMaxProgress(int value)
{
    ui->progressBar->setMaximum(value);
}

void UpdateDialog::updateProgress(int value)
{
    ui->progressBar->setValue(value);
}
