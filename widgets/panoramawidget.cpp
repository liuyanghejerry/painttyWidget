#include "panoramawidget.h"

PanoramaWidget::PanoramaWidget(QWidget *parent) :
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

QSize PanoramaWidget::sizeHint() const
{
    return image_.size() + QSize(10, 10);
}

QSize PanoramaWidget::minimumSizeHint() const
{
    return QSize(144, 96);
}

void PanoramaWidget::onImageChange(const QPixmap &p,
                                   const QRect &r)
{
    full_img_ = p;
    viewport_ = r;
    thumbnail();
    image_ = drawViewport();
    update();
}

void PanoramaWidget::onRectChange(const QRect &r)
{
    viewport_ = r;
    image_ = drawViewport();
    update();
}

QPixmap PanoramaWidget::drawViewport()
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

void PanoramaWidget::thumbnail()
{
    if(full_img_.isNull()){
        return;
    }
    qreal delta = qMin(qreal(preferSize_.width()) / full_img_.width(),
                       qreal(preferSize_.height()) / full_img_.height());
    sized_img_ = full_img_.scaled(full_img_.width() * delta,
                                  full_img_.height() * delta,
                                  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//    if(preferSize_.width() > preferSize_.height()){
//        sized_img_ =
//                full_img_.scaledToHeight(preferSize_.height());
//    }else{
//        sized_img_ =
//                full_img_.scaledToWidth(preferSize_.width());
//    }
}

void PanoramaWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QSize whole = this->size();
    int left = whole.width() - image_.width();
    left /= 2;
    int top = whole.height() - image_.height();
    top /= 2;
    painter.drawPixmap(left, top, image_);
}

void PanoramaWidget::resizeEvent(QResizeEvent * event)
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

void PanoramaWidget::navigateTo(const QPoint &p)
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

void PanoramaWidget::mouseMoveEvent(QMouseEvent * event)
{
    if(event->buttons()){
        navigateTo(event->pos());
    }
}

void PanoramaWidget::mousePressEvent(QMouseEvent * event)
{
    navigateTo(event->pos());
}
