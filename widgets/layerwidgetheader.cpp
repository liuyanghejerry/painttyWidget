#include "layerwidgetheader.h"

#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>

LayerWidgetHeader::LayerWidgetHeader(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    this->setLayout(layout);
    opacity_label_ = new QLabel(this);
    opacity_spinBox_ = new QSpinBox(this);
    opacity_spinBox_->setSuffix(tr("%"));
    opacity_spinBox_->setRange(0,100);


    layout->addWidget(opacity_label_);
    layout->addWidget(opacity_spinBox_);
    layout->addStretch(1);
    opacity_label_->setText(tr("Opacity: "));
}
