#include "developerconsole.h"
#include "ui_developerconsole.h"

DeveloperConsole::DeveloperConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeveloperConsole)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(commandProc()));
}

DeveloperConsole::~DeveloperConsole()
{
    delete ui;
}

void DeveloperConsole::commandProc()
{
    QString commandLine = ui->lineEdit->text();
    ui->lineEdit->clear();
    ui->console->insertPlainText(commandLine);
    ui->console->insertPlainText("\n");
}
