#include "panoramaslider.h"
#include "../../common/common.h"

#include <QHBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QtCore/qmath.h>

using GlobalDef::MIN_SCALE_FACTOR;
using GlobalDef::MAX_SCALE_FACTOR;

//                | slider value - 100  if(slider value >= 0)
// scale factor = |
//                | 100 * 2^(slider value / 100)  if(slider value < 0)

PanoramaSlider::PanoramaSlider(QWidget *parent) :
    QWidget(parent),
    slider(new QSlider(Qt::Horizontal, this)),
    input(new QLineEdit("100%", this)), //too lazy to change this widget to QSpinBox,
                                        //and we don't want to update value instantly (QSpinBox works like this),
                                        //we want to emit signals when enter is pressed (QLineEdit can do this).
    internalFactor(1.0),
    inputReg("([0-9]*)%?") //now we only accept integers, no more floats...
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    input->setFixedWidth(input->fontMetrics().width("100.00%"));
    input->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    input->setValidator(new QRegExpValidator(inputReg, input));

    this->setLayout(layout);
    layout->addWidget(slider);
    layout->addWidget(input);
    layout->setContentsMargins(0, 0, 0, 0);

    internalFactor = -100 * qLn(MIN_SCALE_FACTOR/100.0) / MIN_SCALE_FACTOR / qLn(2);
    slider->setRange(-qCeil(MIN_SCALE_FACTOR * internalFactor),
                     MAX_SCALE_FACTOR - 100);
    slider->setPageStep(100);
    slider->setTickPosition(QSlider::TicksBelow);

    connect(slider, &QSlider::valueChanged, this, &PanoramaSlider::calculateScale);
    connect(input, &QLineEdit::returnPressed, this, &PanoramaSlider::inputScaleConfirmed);
}

void PanoramaSlider::setScale(int scaleFactor)
{
    slider->blockSignals(true);
    if (scaleFactor >= 10)
    {
        int newValue = scaleFactor - 100;
        slider->setValue(newValue);
        input->setText(QString("%1%").arg(scaleFactor));
    }
    else
    {
        int newValue = -internalFactor * MIN_SCALE_FACTOR / qLn(MIN_SCALE_FACTOR/100.0) * qLn(scaleFactor/100.0);
        slider->setValue(newValue);
        input->setText(QString("%1%").arg(scaleFactor));
    }
    slider->blockSignals(false);
}

void PanoramaSlider::calculateScale(int sliderValue)
{
    int newScale;
    if (sliderValue >= 0)
        newScale = 100 + sliderValue;
    else
        newScale = qFloor(100 * qPow(2.0, sliderValue / 100.0));

    input->setText(QString("%1%").arg(newScale));
    emit scaled(newScale);
}

void PanoramaSlider::inputScaleConfirmed()
{
    if (inputReg.indexIn(input->text()))
        calculateScale(slider->value());
    else
    {
        bool ok;
        int newScale = qBound(MIN_SCALE_FACTOR,
                                inputReg.cap(1).toInt(&ok),
                                MAX_SCALE_FACTOR);
        if (ok)
        {
            setScale(newScale);
            emit scaled(newScale);
        }
        else
            calculateScale(slider->value());
    }
}
