#include "colorspinboxgroup.h"
#include "ui_colorspinboxgroup.h"

#include <QDebug>

ColorSpinBoxGroup::ColorSpinBoxGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorSpinBoxGroup),
    isRgbColors(true),
    noColorUpdate(false)
{
    ui->setupUi(this);
    connect(ui->RedspinBox, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
            this, &ColorSpinBoxGroup::onColorChanged);
    connect(ui->GreenspinBox, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
            this, &ColorSpinBoxGroup::onColorChanged);
    connect(ui->BluespinBox, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
            this, &ColorSpinBoxGroup::onColorChanged);
    connect(ui->rgb, &QRadioButton::toggled,
            this, &ColorSpinBoxGroup::onModeChanged);
}

ColorSpinBoxGroup::~ColorSpinBoxGroup()
{
    delete ui;
}

void ColorSpinBoxGroup::setColor(const QColor &c)
{
    if(c == color_) return;
    noColorUpdate = true;
    if(isRgbColors){
        ui->RedspinBox->setValue(c.red());
        ui->GreenspinBox->setValue(c.green());
        ui->BluespinBox->setValue(c.blue());
    }else{
        ui->RedspinBox->setValue(c.hsvHue());
        ui->GreenspinBox->setValue(c.hsvSaturation()*100/255);
        ui->BluespinBox->setValue(c.value()*100/255);
    }
    color_ = c;
    updateLabel();
    noColorUpdate = false;
}

QColor ColorSpinBoxGroup::color()
{
    return color_;
}

void ColorSpinBoxGroup::updateLabel()
{
    QPalette p = ui->label->palette();
    p.setColor(QPalette::Background, color_);
    ui->label->setPalette(p);
}

void ColorSpinBoxGroup::onModeChanged()
{
    bool newValue = ui->rgb->isChecked();
    if(isRgbColors == newValue) return;
    isRgbColors = newValue;
    noColorUpdate = true;
    if(!isRgbColors){
        ui->red->setText(tr("Hue"));
        ui->green->setText(tr("Saturation"));
        ui->blue->setText(tr("Value"));

        ui->RedspinBox->setRange(0,359);
        ui->GreenspinBox->setRange(0,100);
        ui->GreenspinBox->setSuffix("%");
        ui->BluespinBox->setRange(0,100);
        ui->BluespinBox->setSuffix("%");
        color_ = color_.toHsv();
        ui->RedspinBox->setValue(color_.hsvHue());
        ui->GreenspinBox->setValue(color_.hsvSaturation()*100/255);
        ui->BluespinBox->setValue(color_.value()*100/255);
    }else{
        ui->red->setText(tr("Red"));
        ui->green->setText(tr("Green"));
        ui->blue->setText(tr("Blue"));

        ui->RedspinBox->setRange(0,255);
        ui->GreenspinBox->setRange(0,255);
        ui->GreenspinBox->setSuffix("");
        ui->BluespinBox->setRange(0,255);
        ui->BluespinBox->setSuffix("");
        color_ = color_.toRgb();
        ui->RedspinBox->setValue(color_.red());
        ui->GreenspinBox->setValue(color_.green());
        ui->BluespinBox->setValue(color_.blue());
    }
    noColorUpdate = false;
    emit modeChange(isRgbColors);
}

void ColorSpinBoxGroup::onColorChanged()
{
    if(noColorUpdate) return;

    QColor c;
    if(isRgbColors){
        c = QColor::fromRgb(ui->RedspinBox->value(),
                            ui->GreenspinBox->value(),
                            ui->BluespinBox->value()
                            );

    }else{
        c = QColor::fromHsv(qBound(0,
                                   ui->RedspinBox->value(),
                                   359),
                            qBound(0,
                                   ui->GreenspinBox->value(),
                                   100),
                            qBound(0,
                                   ui->BluespinBox->value(),
                                   100)
                            );
    }

    color_ = c;
    updateLabel();
    emit colorChange(c);
}
