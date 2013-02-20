/********************************************************************************
** Form generated from reading UI file 'newroomwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEWROOMWINDOW_H
#define UI_NEWROOMWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_NewRoomWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QCheckBox *checkBox;
    QLineEdit *lineEdit_2;
    QSpinBox *spinBox;
    QLabel *label_2;
    QLabel *label_3;
    QPlainTextEdit *plainTextEdit;
    QCheckBox *checkBox_2;
    QProgressBar *progressBar;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *NewRoomWindow)
    {
        if (NewRoomWindow->objectName().isEmpty())
            NewRoomWindow->setObjectName(QStringLiteral("NewRoomWindow"));
        NewRoomWindow->resize(259, 295);
        NewRoomWindow->setModal(true);
        verticalLayout_2 = new QVBoxLayout(NewRoomWindow);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label = new QLabel(NewRoomWindow);
        label->setObjectName(QStringLiteral("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        lineEdit = new QLineEdit(NewRoomWindow);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setMaxLength(16);

        formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit);

        checkBox = new QCheckBox(NewRoomWindow);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        formLayout->setWidget(3, QFormLayout::LabelRole, checkBox);

        lineEdit_2 = new QLineEdit(NewRoomWindow);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setEnabled(false);
        lineEdit_2->setMaxLength(16);
        lineEdit_2->setEchoMode(QLineEdit::PasswordEchoOnEdit);

        formLayout->setWidget(3, QFormLayout::FieldRole, lineEdit_2);

        spinBox = new QSpinBox(NewRoomWindow);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setMinimum(1);
        spinBox->setMaximum(8);
        spinBox->setValue(4);

        formLayout->setWidget(2, QFormLayout::FieldRole, spinBox);

        label_2 = new QLabel(NewRoomWindow);
        label_2->setObjectName(QStringLiteral("label_2"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_2);


        verticalLayout->addLayout(formLayout);

        label_3 = new QLabel(NewRoomWindow);
        label_3->setObjectName(QStringLiteral("label_3"));

        verticalLayout->addWidget(label_3);

        plainTextEdit = new QPlainTextEdit(NewRoomWindow);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));

        verticalLayout->addWidget(plainTextEdit);

        checkBox_2 = new QCheckBox(NewRoomWindow);
        checkBox_2->setObjectName(QStringLiteral("checkBox_2"));

        verticalLayout->addWidget(checkBox_2);

        progressBar = new QProgressBar(NewRoomWindow);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setValue(0);

        verticalLayout->addWidget(progressBar);


        verticalLayout_2->addLayout(verticalLayout);

        buttonBox = new QDialogButtonBox(NewRoomWindow);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);

        QWidget::setTabOrder(lineEdit, spinBox);
        QWidget::setTabOrder(spinBox, checkBox);
        QWidget::setTabOrder(checkBox, lineEdit_2);
        QWidget::setTabOrder(lineEdit_2, plainTextEdit);
        QWidget::setTabOrder(plainTextEdit, checkBox_2);
        QWidget::setTabOrder(checkBox_2, buttonBox);

        retranslateUi(NewRoomWindow);
        QObject::connect(checkBox, SIGNAL(toggled(bool)), lineEdit_2, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(NewRoomWindow);
    } // setupUi

    void retranslateUi(QDialog *NewRoomWindow)
    {
        NewRoomWindow->setWindowTitle(QApplication::translate("NewRoomWindow", "New Room", 0));
        label->setText(QApplication::translate("NewRoomWindow", "Room Name*", 0));
        checkBox->setText(QApplication::translate("NewRoomWindow", "Use Password", 0));
        label_2->setText(QApplication::translate("NewRoomWindow", "Max Member*", 0));
        label_3->setText(QApplication::translate("NewRoomWindow", "Welome Words", 0));
        checkBox_2->setText(QApplication::translate("NewRoomWindow", "Auto close room when everyone leaves", 0));
    } // retranslateUi

};

namespace Ui {
    class NewRoomWindow: public Ui_NewRoomWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEWROOMWINDOW_H
