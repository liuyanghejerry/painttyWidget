#include "brushsettingswidget.h"
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

BrushSettingsWidget::BrushSettingsWidget(QWidget *parent) :
    QWidget(parent),
    layout_(nullptr)
{
    widthLabel = new QLabel(tr("Width"), this);
    widthLabel->setAlignment(Qt::AlignCenter);
    widthSlider = new QSlider(this);
    widthSpinBox = new QSpinBox(this);
    separator = new QFrame(this);
    hardnessLabel = new QLabel(tr("Hardness"), this);
    hardnessSlider = new QSlider(this);
    hardnessSpinBox = new QSpinBox(this);

    widthSlider->setSizePolicy(QSizePolicy::Preferred,
                               QSizePolicy::Fixed);
    widthSlider->setRange(1, 100);
    widthSpinBox->setRange(1, 100);
    hardnessLabel->setAlignment(Qt::AlignCenter);
    separator->setFrameShadow(QFrame::Sunken);
    hardnessSlider->setSizePolicy(QSizePolicy::Preferred,
                                  QSizePolicy::Fixed);
    hardnessSlider->setRange(0, 100);
    hardnessSpinBox->setRange(0, 100);

    connect(widthSlider, &QSlider::valueChanged,
            this, &BrushSettingsWidget::widthChanged);
    connect(widthSlider, &QSlider::valueChanged,
            widthSpinBox, &QSpinBox::setValue);
    connect(widthSpinBox,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            widthSlider, &QSlider::setValue);
    connect(hardnessSlider, &QSlider::valueChanged,
            this, &BrushSettingsWidget::hardnessChanged);
    connect(hardnessSlider, &QSlider::valueChanged,
            hardnessSpinBox, &QSpinBox::setValue);
    connect(hardnessSpinBox,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            hardnessSlider, &QSlider::setValue);

    setOrientation(Qt::Horizontal);
    setHardness(100);
    setWidth(10);
}

BrushSettingsWidget::~BrushSettingsWidget()
{
}

int BrushSettingsWidget::width()
{
    return widthSlider->value();
}

void BrushSettingsWidget::setWidth(int width)
{
    if(width != widthSlider->value())
        widthSlider->setValue(width);
}

int BrushSettingsWidget::hardness()
{
    return hardnessSlider->value();
}

void BrushSettingsWidget::setHardness(int hardness)
{
    if (hardness != hardnessSlider->value())
        hardnessSlider->setValue(hardness);
}

void BrushSettingsWidget::widthUp()
{
    widthSlider->setValue(widthSlider->value()
                          + widthSlider->singleStep());
}

void BrushSettingsWidget::widthDown()
{
    widthSlider->setValue(widthSlider->value()
                          - widthSlider->singleStep());
}

void BrushSettingsWidget::hardnessUp()
{
    hardnessSlider->setValue(hardnessSlider->value()
                             + hardnessSlider->singleStep());
}

void BrushSettingsWidget::hardnessDown()
{
    hardnessSlider->setValue(hardnessSlider->value()
                             - hardnessSlider->singleStep());
}

void BrushSettingsWidget::setOrientation(Qt::Orientation ori)
{
    if(layout_){
        delete layout_;
    }
    if(ori == Qt::Horizontal){
        layout_ = new QHBoxLayout(this);
        separator->setFrameShape(QFrame::HLine);
    }else{
        layout_ = new QVBoxLayout(this);
        separator->setFrameShape(QFrame::VLine);
    }
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setAlignment(Qt::AlignCenter);
    layout_->addWidget(widthLabel);
    layout_->addWidget(widthSlider);
    layout_->addWidget(widthSpinBox);
    layout_->addWidget(separator);
    layout_->addWidget(hardnessLabel);
    layout_->addWidget(hardnessSlider);
    layout_->addWidget(hardnessSpinBox);
    widthSlider->setOrientation(ori);
    hardnessSlider->setOrientation(ori);
    setLayout(layout_);
}
