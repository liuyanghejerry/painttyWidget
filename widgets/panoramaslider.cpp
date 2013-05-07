#include "panoramaslider.h"
#include "../../common/common.h"

#include <QHBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QtCore/qmath.h>

using GlobalDef::MIN_SCALE_FACTOR;
using GlobalDef::MAX_SCALE_FACTOR;

PanoramaSlider::PanoramaSlider(QWidget *parent) :
    QWidget(parent),
    slider(new QSlider(Qt::Horizontal, this)),
    input(new QLineEdit("100%", this)),
    internalFactor(1.0),
    inputReg("([0-9]*(\\.[0-9]*)?)%?")
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    input->setFixedWidth(input->fontMetrics().width("100.00%"));
    input->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    input->setValidator(new QRegExpValidator(inputReg, input));

    this->setLayout(layout);
    layout->addWidget(slider);
    layout->addWidget(input);
    layout->setContentsMargins(0, 0, 0, 0);

    internalFactor = -100 * qLn(MIN_SCALE_FACTOR) / MIN_SCALE_FACTOR / qLn(2);
    slider->setRange(-qCeil(MIN_SCALE_FACTOR * internalFactor),
                     qCeil(MAX_SCALE_FACTOR - 1) * 100);
    slider->setPageStep(100);
    slider->setTickPosition(QSlider::TicksBelow);

    connect(slider, &QSlider::valueChanged, this, &PanoramaSlider::calculateScale);
    connect(input, &QLineEdit::returnPressed, this, &PanoramaSlider::inputScaleConfirmed);
}

void PanoramaSlider::setScale(qreal scaleFactor)
{
    slider->blockSignals(true);
    if (scaleFactor >= 1.0)
    {
        int newValue = 100 * scaleFactor - 100;
        slider->setValue(newValue);
        input->setText(QString("%1%").arg(qFloor(100 * scaleFactor)));
    }
    else
    {
        int newValue = -internalFactor * MIN_SCALE_FACTOR / qLn(MIN_SCALE_FACTOR) * qLn(scaleFactor);
        slider->setValue(newValue);
        input->setText(QString("%1%").arg(100 * scaleFactor, 0, 'f', 1));
    }
    slider->blockSignals(false);
}

void PanoramaSlider::calculateScale(int sliderValue)
{
    if (sliderValue >= 0)
    {
        qreal newScale = 1.0 + 0.01 * sliderValue;
        input->setText(QString("%1%")
                       .arg(100 + sliderValue));
        emit scaled(newScale);
    }
    else
    {
        qreal newScale = qPow(MIN_SCALE_FACTOR,
                              -sliderValue
                              / internalFactor
                              / MIN_SCALE_FACTOR);
        input->setText(QString("%1%")
                       .arg(newScale * 100, 0, 'f', 1));
        emit scaled(newScale);
    }
}

void PanoramaSlider::inputScaleConfirmed()
{
    if (inputReg.indexIn(input->text()))
        calculateScale(slider->value());
    else
    {
        bool ok;
        qreal newScale = qBound(MIN_SCALE_FACTOR,
                                inputReg.cap(1).toDouble(&ok) / 100.0,
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
