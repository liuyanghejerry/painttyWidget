#include "developerconsole.h"
#include "ui_developerconsole.h"
#include <QDebug>
#include <QScrollBar>

DeveloperConsole::DeveloperConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeveloperConsole)
{
    ui->setupUi(this);
    connect(ui->pushButton,&QPushButton::clicked,
            this, &DeveloperConsole::commandProc);
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

    QTextCursor c = ui->console->textCursor();
    c.movePosition(QTextCursor::End);
    ui->console->setTextCursor(c);

    ui->console->insertPlainText(QString("%1%2%3")
                                 .arg(">")
                                 .arg(commandLine)
                                 .arg("\n"));
    ui->console->verticalScrollBar()
            ->setValue(ui->console->verticalScrollBar()
                       ->maximum());

    emit evaluate(commandLine);
}

void DeveloperConsole::append(const QString &content)
{
    QTextCursor c = ui->console->textCursor();
    c.movePosition(QTextCursor::End);
    ui->console->setTextCursor(c);

    ui->console->insertPlainText(content+"\n");

    ui->console->verticalScrollBar()
            ->setValue(ui->console->verticalScrollBar()
                       ->maximum());


}
