/********************************************************************************
** Form generated from reading UI file 'roomlistdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROOMLISTDIALOG_H
#define UI_ROOMLISTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_RoomListDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *tableWidget;
    QCheckBox *checkBox;
    QProgressBar *progressBar;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QLabel *label;

    void setupUi(QDialog *RoomListDialog)
    {
        if (RoomListDialog->objectName().isEmpty())
            RoomListDialog->setObjectName(QStringLiteral("RoomListDialog"));
        RoomListDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(RoomListDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tableWidget = new QTableWidget(RoomListDialog);
        tableWidget->setObjectName(QStringLiteral("tableWidget"));
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableWidget->setSortingEnabled(true);
        tableWidget->setCornerButtonEnabled(false);
        tableWidget->verticalHeader()->setVisible(false);

        verticalLayout->addWidget(tableWidget);

        checkBox = new QCheckBox(RoomListDialog);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        verticalLayout->addWidget(checkBox);

        progressBar = new QProgressBar(RoomListDialog);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setValue(0);

        verticalLayout->addWidget(progressBar);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(RoomListDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        lineEdit = new QLineEdit(RoomListDialog);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setMaxLength(32767);

        horizontalLayout_2->addWidget(lineEdit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(RoomListDialog);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(RoomListDialog);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

        horizontalLayout->addWidget(pushButton_2);

        pushButton_3 = new QPushButton(RoomListDialog);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));

        horizontalLayout->addWidget(pushButton_3);

        pushButton_4 = new QPushButton(RoomListDialog);
        pushButton_4->setObjectName(QStringLiteral("pushButton_4"));

        horizontalLayout->addWidget(pushButton_4);


        verticalLayout->addLayout(horizontalLayout);

        label = new QLabel(RoomListDialog);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);


        retranslateUi(RoomListDialog);

        QMetaObject::connectSlotsByName(RoomListDialog);
    } // setupUi

    void retranslateUi(QDialog *RoomListDialog)
    {
        RoomListDialog->setWindowTitle(QApplication::translate("RoomListDialog", "Mr.Paint", 0));
        checkBox->setText(QApplication::translate("RoomListDialog", "Don't show it if room is full", 0));
        label_2->setText(QApplication::translate("RoomListDialog", "Nick Name:", 0));
        lineEdit->setInputMask(QString());
        lineEdit->setText(QApplication::translate("RoomListDialog", "Someone", 0));
        pushButton->setText(QApplication::translate("RoomListDialog", "New", 0));
        pushButton_2->setText(QApplication::translate("RoomListDialog", "Join", 0));
        pushButton_3->setText(QApplication::translate("RoomListDialog", "Refresh", 0));
        pushButton_4->setText(QApplication::translate("RoomListDialog", "Exit", 0));
        label->setText(QApplication::translate("RoomListDialog", "Note: We're still in alpha test, and only provide 50 rooms.", 0));
    } // retranslateUi

};

namespace Ui {
    class RoomListDialog: public Ui_RoomListDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROOMLISTDIALOG_H
