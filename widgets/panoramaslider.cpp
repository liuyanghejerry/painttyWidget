#include "panoramaslider.h"
#include "../common.h"

#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QtCore/qmath.h>

using GlobalDef::MIN_SCALE_FACTOR;
using GlobalDef::MAX_SCALE_FACTOR;

PanoramaSlider::PanoramaSlider(QWidget *parent) :
    QWidget(parent)
    //currentScaleFactor_(1.0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    slider = new QSlider(Qt::Horizontal, this);
    label = new QLabel("100%", this);
    label->setFixedWidth(label->fontMetrics().width("100.%"));
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    //label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); //not working?

    this->setLayout(layout);
    layout->addWidget(slider);
    layout->addWidget(label);
    layout->setContentsMargins(0, 0, 0, 0);

    internalFactor = -100 * qLn(MIN_SCALE_FACTOR) / MIN_SCALE_FACTOR / qLn(2);
    slider->setRange(-qCeil(MIN_SCALE_FACTOR * internalFactor),
                     qCeil(MAX_SCALE_FACTOR - 1) * 100);
    slider->setPageStep(100);
    slider->setTickPosition(QSlider::TicksBelow);

    auto calculateScale = [this](int sliderValue){
        if (sliderValue >= 0){
            qreal newScale = 1.0 + 0.01 * sliderValue;
            label->setText(QString("%1%")
                           .arg(100 + sliderValue));
            emit scaled(newScale);
        }else{
            qreal newScale = qPow(MIN_SCALE_FACTOR,
                                  -sliderValue
                                  / internalFactor
                                  / MIN_SCALE_FACTOR);
            label->setText(QString("%1%")
                           .arg(newScale * 100, 0, 'f', 1));
            emit scaled(newScale);
        }
    };
//    auto calculateValue = [this, slider, label, internalFactor](qreal scaleFactor){
//        slider->blockSignals(true);

//        slider->blockSignals(false);
//    };

    connect(slider, &QSlider::valueChanged, calculateScale);
    //connect(this, &PanoramaSlider::scaled, calculateValue);
}

void PanoramaSlider::setScale(qreal scaleFactor)
{
    slider->blockSignals(true);
    if (scaleFactor >= 1.0){
        int newValue = 100 * scaleFactor - 100;
        if (newValue != slider->value()){
            slider->setValue(newValue);
            label->setText(QString("%1%").arg(qFloor(100 * scaleFactor)));
        }
    }else{
        int newValue = -internalFactor * MIN_SCALE_FACTOR / qLn(MIN_SCALE_FACTOR) * qLn(scaleFactor);
        if (newValue != slider->value()){
            slider->setValue(newValue);
            label->setText(QString("%1%").arg(100 * scaleFactor, 0, 'f', 1));
        }
    }
    slider->blockSignals(false);
}

//qreal PanoramaSlider::currentScaleFactor()
//{
//    return currentScaleFactor_;
//}

