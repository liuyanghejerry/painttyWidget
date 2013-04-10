#include "panoramawidget.h"
#include "panoramaslider.h"

#include <QVBoxLayout>

PanoramaWidget::PanoramaWidget(QWidget *parent) :
    QWidget(parent)
{
    connect(&view, &PanoramaView::refresh,
            this, &PanoramaWidget::refresh);
    connect(&view, &PanoramaView::moveTo,
            this, &PanoramaWidget::moveTo);
    connect(&view, &PanoramaView::viewportChange,
            this, &PanoramaWidget::viewportChange);
    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);
    layout->addWidget(new PanoramaView(this));
    layout->addWidget(new PanoramaSlider(this));
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(2);
}

void PanoramaWidget::onImageChange(const QPixmap &p,
                                   const QRect &r)
{
    view.onImageChange(p,r);
}

void PanoramaWidget::onRectChange(const QRect &r)
{
    view.onRectChange(r);
}
