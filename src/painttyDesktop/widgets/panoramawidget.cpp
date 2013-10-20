#include "panoramawidget.h"
#include "panoramaslider.h"
#include "panoramaview.h"

#include <QVBoxLayout>

PanoramaWidget::PanoramaWidget(QWidget *parent) :
    QWidget(parent),
    slider(new PanoramaSlider(this)),
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

    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(slider);
    layout->setStretchFactor(view, 1);
    layout->setStretchFactor(slider, 0);
    layout->setSpacing(2);
}

void PanoramaWidget::setScaled(qreal f)
{
    if(slider)
        slider->setScale(f);
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
