#include "brushwidthwidget.h"
#include "ui_brushwidthwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

BrushWidthWidget::BrushWidthWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrushWidthWidget),
    layout_(nullptr)
{
    ui->setupUi(this);
    setOrientation(Qt::Horizontal);
    ui->slider->setSizePolicy(QSizePolicy::Preferred,
                              QSizePolicy::Preferred);
    ui->slider->setRange(1, 100);
    connect(ui->slider, &QSlider::valueChanged,
            [&](int value){
        ui->width_show->setText(tr("%1 px").arg(value));
        emit valueChanged(value);
    });
}

BrushWidthWidget::~BrushWidthWidget()
{
    delete ui;
}

int BrushWidthWidget::value()
{
    return ui->slider->value();
}

void BrushWidthWidget::setValue(int v)
{
    if(v != ui->slider->value()){
        ui->slider->setValue(v);
    }
}

void BrushWidthWidget::setStep(int v)
{
    ui->slider->setSingleStep(v);
}

void BrushWidthWidget::up()
{
    ui->slider->setValue(ui->slider->value()
                         + ui->slider->singleStep());
}

void BrushWidthWidget::down()
{
    ui->slider->setValue(ui->slider->value()
                         - ui->slider->singleStep());
}

void BrushWidthWidget::setOrientation(Qt::Orientation ori)
{
    this->setLayout(0);
    if(layout_){
        delete layout_;
    }
    if(ori == Qt::Horizontal){
        layout_ = new QHBoxLayout(this);
    }else{
        layout_ = new QVBoxLayout(this);
    }
    layout_->addWidget(ui->label);
    layout_->addWidget(ui->slider);
    layout_->addWidget(ui->width_show);
    ui->slider->setOrientation(ori);
}
