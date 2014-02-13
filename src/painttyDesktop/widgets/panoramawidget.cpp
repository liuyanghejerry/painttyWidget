#include "panoramawidget.h"
#include "panoramaslider.h"
#include "panoramarotator.h"
#include "panoramaview.h"

#include <QVBoxLayout>

PanoramaWidget::PanoramaWidget(QWidget *parent) :
    QWidget(parent),
    slider(new PanoramaSlider(this)),
    rotator(new PanoramaRotator(this)),
    view(new PanoramaView(this))
{
    connect(view, &PanoramaView::refresh,
            this, &PanoramaWidget::refresh);
    connect(view, &PanoramaView::moveTo,
            this, &PanoramaWidget::moveTo);
    connect(view, &PanoramaView::viewportChange,
            this, &PanoramaWidget::viewportChange);
    connect(this, &PanoramaWidget::setScaled,
            slider, &PanoramaSlider::setScale);
    connect(slider, &PanoramaSlider::scaled,
            this, &PanoramaWidget::scaled);
    connect(this, &PanoramaWidget::setRotation,
            rotator, &PanoramaRotator::setRotation);
    connect(rotator, &PanoramaRotator::rotated,
            this, &PanoramaWidget::rotated);

    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(slider);
    layout->addWidget(rotator);
    layout->setStretchFactor(view, 1);
    layout->setStretchFactor(slider, 0);
    layout->setStretchFactor(rotator, 0);
    layout->setSpacing(2);
}

void PanoramaWidget::setScaled(qreal f)
{
    if(slider)
        slider->setScale(f);
}

void PanoramaWidget::setRotation(int d)
{
    if(rotator){
        rotator->setRotation(d);
    }
}

void PanoramaWidget::onImageChange(const QPixmap &p,
                                   const QRect &r)
{
    if(view)
        view->onImageChange(p,r);
}

void PanoramaWidget::onRectChange(const QRect &r)
{
    if(view)
        view->onRectChange(r);
}
