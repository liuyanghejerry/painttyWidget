/********************************************************************************
** Form generated from reading UI file 'colorspinboxgroup.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORSPINBOXGROUP_H
#define UI_COLORSPINBOXGROUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ColorSpinBoxGroup
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QRadioButton *hsv;
    QRadioButton *rgb;
    QGridLayout *gridLayout;
    QSpinBox *GreenspinBox;
    QLabel *blue;
    QSpinBox *RedspinBox;
    QLabel *green;
    QLabel *red;
    QSpinBox *BluespinBox;
    QLabel *label;

    void setupUi(QWidget *ColorSpinBoxGroup)
    {
        if (ColorSpinBoxGroup->objectName().isEmpty())
            ColorSpinBoxGroup->setObjectName(QStringLiteral("ColorSpinBoxGroup"));
        ColorSpinBoxGroup->resize(204, 128);
        verticalLayout = new QVBoxLayout(ColorSpinBoxGroup);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        hsv = new QRadioButton(ColorSpinBoxGroup);
        hsv->setObjectName(QStringLiteral("hsv"));

        horizontalLayout->addWidget(hsv);

        rgb = new QRadioButton(ColorSpinBoxGroup);
        rgb->setObjectName(QStringLiteral("rgb"));
        rgb->setChecked(true);

        horizontalLayout->addWidget(rgb);


        verticalLayout->addLayout(horizontalLayout);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        GreenspinBox = new QSpinBox(ColorSpinBoxGroup);
        GreenspinBox->setObjectName(QStringLiteral("GreenspinBox"));
        GreenspinBox->setMaximum(255);

        gridLayout->addWidget(GreenspinBox, 1, 1, 1, 1);

        blue = new QLabel(ColorSpinBoxGroup);
        blue->setObjectName(QStringLiteral("blue"));

        gridLayout->addWidget(blue, 2, 0, 1, 1);

        RedspinBox = new QSpinBox(ColorSpinBoxGroup);
        RedspinBox->setObjectName(QStringLiteral("RedspinBox"));
        RedspinBox->setMaximum(255);

        gridLayout->addWidget(RedspinBox, 0, 1, 1, 1);

        green = new QLabel(ColorSpinBoxGroup);
        green->setObjectName(QStringLiteral("green"));

        gridLayout->addWidget(green, 1, 0, 1, 1);

        red = new QLabel(ColorSpinBoxGroup);
        red->setObjectName(QStringLiteral("red"));

        gridLayout->addWidget(red, 0, 0, 1, 1);

        BluespinBox = new QSpinBox(ColorSpinBoxGroup);
        BluespinBox->setObjectName(QStringLiteral("BluespinBox"));
        BluespinBox->setMaximum(255);

        gridLayout->addWidget(BluespinBox, 2, 1, 1, 1);

        label = new QLabel(ColorSpinBoxGroup);
        label->setObjectName(QStringLiteral("label"));
        label->setAutoFillBackground(true);
        label->setFrameShape(QFrame::Box);
        label->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(label, 0, 2, 3, 1);


        verticalLayout->addLayout(gridLayout);

        QWidget::setTabOrder(hsv, rgb);
        QWidget::setTabOrder(rgb, RedspinBox);
        QWidget::setTabOrder(RedspinBox, GreenspinBox);
        QWidget::setTabOrder(GreenspinBox, BluespinBox);

        retranslateUi(ColorSpinBoxGroup);

        QMetaObject::connectSlotsByName(ColorSpinBoxGroup);
    } // setupUi

    void retranslateUi(QWidget *ColorSpinBoxGroup)
    {
        hsv->setText(QApplication::translate("ColorSpinBoxGroup", "HSV", 0));
        rgb->setText(QApplication::translate("ColorSpinBoxGroup", "RGB", 0));
        blue->setText(QApplication::translate("ColorSpinBoxGroup", "Blue", 0));
        green->setText(QApplication::translate("ColorSpinBoxGroup", "Green", 0));
        red->setText(QApplication::translate("ColorSpinBoxGroup", "Red", 0));
        Q_UNUSED(ColorSpinBoxGroup);
    } // retranslateUi

};

namespace Ui {
    class ColorSpinBoxGroup: public Ui_ColorSpinBoxGroup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORSPINBOXGROUP_H
