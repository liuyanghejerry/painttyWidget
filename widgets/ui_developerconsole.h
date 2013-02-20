/********************************************************************************
** Form generated from reading UI file 'developerconsole.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVELOPERCONSOLE_H
#define UI_DEVELOPERCONSOLE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DeveloperConsole
{
public:
    QVBoxLayout *verticalLayout;
    QTextEdit *console;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit;
    QPushButton *pushButton;

    void setupUi(QDialog *DeveloperConsole)
    {
        if (DeveloperConsole->objectName().isEmpty())
            DeveloperConsole->setObjectName(QStringLiteral("DeveloperConsole"));
        DeveloperConsole->resize(400, 300);
        verticalLayout = new QVBoxLayout(DeveloperConsole);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        console = new QTextEdit(DeveloperConsole);
        console->setObjectName(QStringLiteral("console"));
        console->setStyleSheet(QLatin1String("#console {\n"
"	color: white;\n"
"	background-color: black;\n"
"	font: 75 11pt \"Courier New\";\n"
"}"));
        console->setLineWrapMode(QTextEdit::NoWrap);
        console->setReadOnly(true);
        console->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);

        verticalLayout->addWidget(console);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        lineEdit = new QLineEdit(DeveloperConsole);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        pushButton = new QPushButton(DeveloperConsole);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(DeveloperConsole);
        QObject::connect(lineEdit, SIGNAL(returnPressed()), pushButton, SLOT(click()));

        QMetaObject::connectSlotsByName(DeveloperConsole);
    } // setupUi

    void retranslateUi(QDialog *DeveloperConsole)
    {
        DeveloperConsole->setWindowTitle(QApplication::translate("DeveloperConsole", "Dialog", 0));
        lineEdit->setPlaceholderText(QApplication::translate("DeveloperConsole", "Type command and push...", 0));
        pushButton->setText(QApplication::translate("DeveloperConsole", "Push", 0));
    } // retranslateUi

};

namespace Ui {
    class DeveloperConsole: public Ui_DeveloperConsole {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVELOPERCONSOLE_H
