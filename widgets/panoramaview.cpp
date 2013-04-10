#include "panoramaview.h"

#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>
#include <QDebug>

PanoramaView::PanoramaView(QWidget *parent) :
    QWidget(parent),
    preferSize_(144, 96),
    image_( preferSize_ ),
    full_img_(preferSize_),
    sized_img_(preferSize_)
{
    QPalette p = this->palette();
    p.setColor(QPalette::Background, Qt::gray);
    this->setPalette(p);
    sized_img_.fill(Qt::white);
    full_img_.fill(Qt::white);
    image_.fill(Qt::white);
    timer_.setInterval(5*1000);
    connect(&timer_, SIGNAL(timeout()),
            this, SIGNAL(refresh()));
    timer_.start();
    emit refresh();
}

QSize PanoramaView::sizeHint() const
{
    return image_.size();
}

QSize PanoramaView::minimumSizeHint() const
{
    return QSize(144, 96);
}

void PanoramaView::onImageChange(const QPixmap &p,
                                   const QRect &r)
{
    full_img_ = p;
    viewport_ = r;
    thumbnail();
    image_ = drawViewport();
    update();
}

void PanoramaView::onRectChange(const QRect &r)
{
    viewport_ = r;
    image_ = drawViewport();
    update();
}

QPixmap PanoramaView::drawViewport()
{
    QPixmap p = sized_img_;
    if(p.isNull()){
        return p;
    }
    const QRect &r = viewport_;
    QPainter painter(&p);
    QPen pen;
    pen.setColor(this->palette().color(QPalette::Text));
    pen.setWidth(1);
    painter.setPen(pen);
    qreal delta = p.width()/qreal(full_img_.width());
    QPointF topleft(r.topLeft() * delta);
    QRect thumbRect = QRectF(topleft,
                             r.size() * delta).toRect();
    thumbRect.setRight(qMin(thumbRect.right(), p.width() - 2));
    thumbRect.setBottom(qMin(thumbRect.bottom(), p.height() - 2));

    painter.drawRect(thumbRect);
    return p;
}

void PanoramaView::thumbnail()
{
    if(full_img_.isNull()){
        return;
    }
    qreal delta = qMin(qreal(preferSize_.width()) / full_img_.width(),
                       qreal(preferSize_.height()) / full_img_.height());
    sized_img_ = full_img_.scaled(full_img_.width() * delta,
                                  full_img_.height() * delta,
                                  Qt::IgnoreAspectRatio,
                                  Qt::SmoothTransformation);
}

void PanoramaView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QSize whole = this->size();
    int left = whole.width() - image_.width();
    left /= 2;
    int top = whole.height() - image_.height();
    top /= 2;
    painter.drawPixmap(left, top, image_);
}

void PanoramaView::resizeEvent(QResizeEvent * event)
{
    preferSize_ = event->size();
    if(image_.isNull()){
        return;
    }
    thumbnail();
    image_ = drawViewport();
    if(this->isVisible()){
        update();
    }
}

void PanoramaView::navigateTo(const QPoint &p)
{
    QSize whole = this->size();
    int left = whole.width() - image_.width();
    left /= 2;
    int top = whole.height() - image_.height();
    top /= 2;

    qreal delta = qreal(full_img_.width())/sized_img_.width();
    QPointF miniPoint(p-QPoint(left, top));
    QPointF realPoint = miniPoint * delta;

    emit moveTo(realPoint);
}

void PanoramaView::mouseMoveEvent(QMouseEvent * event)
{
    if(event->buttons()){
        navigateTo(event->pos());
    }
}

void PanoramaView::mousePressEvent(QMouseEvent * event)
{
    navigateTo(event->pos());
}
