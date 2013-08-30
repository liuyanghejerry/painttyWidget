#include "developerconsole.h"
#include "ui_developerconsole.h"
#include <QDebug>

DeveloperConsole::DeveloperConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeveloperConsole)
{
    ui->setupUi(this);
    connect(ui->pushButton,&QPushButton::clicked,
            this, &DeveloperConsole::commandProc);
    setWindowTitle(tr("Developer Console"));
}

DeveloperConsole::~DeveloperConsole()
{
    delete ui;
}

void DeveloperConsole::commandProc()
{
    QString commandLine = ui->lineEdit->text();
    ui->lineEdit->clear();
    if(commandLine.isEmpty()){
        return;
    }
    ui->console->insertPlainText(QString("%1%2%3")
                                 .arg(">")
                                 .arg(commandLine)
                                 .arg("\n"));
    emit evaluate(commandLine);
}

void DeveloperConsole::append(const QString &content)
{
    ui->console->insertPlainText(content+"\n");
}
