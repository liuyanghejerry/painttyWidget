/********************************************************************************
** Form generated from reading UI file 'colorbox.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORBOX_H
#define UI_COLORBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "colorspinboxgroup.h"
#include "colorwheel.h"

QT_BEGIN_NAMESPACE

class Ui_ColorBox
{
public:
    QVBoxLayout *verticalLayout;
    ColorWheel *colorWheel;
    ColorSpinBoxGroup *colorSpinBoxGroup;

    void setupUi(QWidget *ColorBox)
    {
        if (ColorBox->objectName().isEmpty())
            ColorBox->setObjectName(QStringLiteral("ColorBox"));
        ColorBox->resize(265, 132);
        verticalLayout = new QVBoxLayout(ColorBox);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        colorWheel = new ColorWheel(ColorBox);
        colorWheel->setObjectName(QStringLiteral("colorWheel"));

        verticalLayout->addWidget(colorWheel);

        colorSpinBoxGroup = new ColorSpinBoxGroup(ColorBox);
        colorSpinBoxGroup->setObjectName(QStringLiteral("colorSpinBoxGroup"));

        verticalLayout->addWidget(colorSpinBoxGroup);


        retranslateUi(ColorBox);

        QMetaObject::connectSlotsByName(ColorBox);
    } // setupUi

    void retranslateUi(QWidget *ColorBox)
    {
        ColorBox->setWindowTitle(QApplication::translate("ColorBox", "Form", 0));
#ifndef QT_NO_TOOLTIP
        colorSpinBoxGroup->setToolTip(QApplication::translate("ColorBox", "Color SpinBox Group", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_WHATSTHIS
        colorSpinBoxGroup->setWhatsThis(QApplication::translate("ColorBox", "The color spin box group contains spin box about colors.", 0));
#endif // QT_NO_WHATSTHIS
    } // retranslateUi

};

namespace Ui {
    class ColorBox: public Ui_ColorBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORBOX_H
