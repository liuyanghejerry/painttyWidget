#include "brushwidthwidget.h"
//#include "ui_brushwidthwidget.h"
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

BrushWidthWidget::BrushWidthWidget(QWidget *parent) :
    QWidget(parent),
    layout_(nullptr)
{
    widthLabel = new QLabel(tr("Width"), this);
    widthSlider = new QSlider(this);
    widthSpinBox = new QSpinBox(this);
    unitLabel = new QLabel(tr("px"), this);

    widthSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    widthSlider->setRange(1, 100);
    widthSpinBox->setRange(1, 100);

    connect(widthSlider, &QSlider::valueChanged, this, &BrushWidthWidget::valueChanged);
    connect(widthSlider, &QSlider::valueChanged, widthSpinBox, &QSpinBox::setValue);
    connect(widthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            widthSlider, &QSlider::setValue);

    setOrientation(Qt::Horizontal);
}

BrushWidthWidget::~BrushWidthWidget()
{
}

int BrushWidthWidget::value()
{
    return widthSlider->value();
}

void BrushWidthWidget::setValue(int v)
{
    if(v != widthSlider->value()){
        widthSlider->setValue(v);
    }
}

void BrushWidthWidget::setStep(int v)
{
    widthSlider->setSingleStep(v);
}

void BrushWidthWidget::up()
{
    widthSlider->setValue(widthSlider->value()
                          + widthSlider->singleStep());
}

void BrushWidthWidget::down()
{
    widthSlider->setValue(widthSlider->value()
                          - widthSlider->singleStep());
}

void BrushWidthWidget::setOrientation(Qt::Orientation ori)
{
    if(layout_){
        delete layout_;
    }
    if(ori == Qt::Horizontal){
        layout_ = new QHBoxLayout(this);
    }else{
        layout_ = new QVBoxLayout(this);
    }
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(widthLabel);
    layout_->addWidget(widthSlider);
    layout_->addWidget(widthSpinBox);
    layout_->addWidget(unitLabel);
    widthSlider->setOrientation(ori);
    setLayout(layout_);
}
