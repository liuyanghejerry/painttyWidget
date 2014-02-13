#include "panoramarotator.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QSlider>
#include <QIntValidator>

PanoramaRotator::PanoramaRotator(QWidget *parent) :
    QWidget(parent),
    lineEdit_(new QLineEdit("0", this)),
    slider_(new QSlider(Qt::Horizontal, this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    lineEdit_->setFixedWidth(lineEdit_->fontMetrics().width("100.00%"));
    lineEdit_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QIntValidator *inv = new QIntValidator(-180, 180, this);
    lineEdit_->setValidator(inv);

    this->setLayout(layout);
    layout->addWidget(slider_);
    layout->addWidget(lineEdit_);
    layout->setContentsMargins(0, 0, 0, 0);

    slider_->setRange(-180, 180);
    slider_->setPageStep(10);
    slider_->setTickPosition(QSlider::TicksBelow);

    connect(slider_, &QSlider::valueChanged, this, &PanoramaRotator::setRotation);
    connect(slider_, &QSlider::valueChanged, [this](int v){
        lineEdit_->setText(QString::number(v));
    });
    connect(lineEdit_, &QLineEdit::textChanged,
            [this](const QString &v){
        this->setRotation(v.toInt());
    });
}

void PanoramaRotator::setRotation(int deg)
{
    if(deg == slider_->value()){
        return;
    }
    slider_->setValue(deg);
    emit rotated(deg);
}
