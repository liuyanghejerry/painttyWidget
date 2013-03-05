#include "panoramawidget.h"

PanoramaWidget::PanoramaWidget(QWidget *parent) :
    QWidget(parent),
    preferSize_(144, 96),
    image_( preferSize_ )
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
}

QSize PanoramaWidget::sizeHint() const
{
    return image_.size() + QSize(10, 10);
}

QSize PanoramaWidget::minimumSizeHint() const
{
    return QSize(144, 96);
}

void PanoramaWidget::onImageChange(QPixmap p,
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
    const QRect &r = viewport_;
    QPainter painter(&p);
    QPen pen;
    pen.setColor(this->palette().color(QPalette::Text));
    pen.setWidth(2);
    painter.setPen(pen);
    qreal delta = p.width()/qreal(full_img_.width());
    QPointF topleft(r.topLeft().x() * delta + 1.0,
                    r.topLeft().y() * delta + 1.0);
    QSizeF thumbSize(r.width() *delta,
                     r.height() *delta);
    QRect thumbRect = QRectF(topleft,
                             thumbSize).toRect();

    painter.drawRect(thumbRect);
    return p;
}

void PanoramaWidget::thumbnail()
{
    if(preferSize_.width() > preferSize_.height()){
        sized_img_ =
                full_img_.scaledToHeight(preferSize_.height());
    }else{
        sized_img_ =
                full_img_.scaledToWidth(preferSize_.width());
    }
}

void PanoramaWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QSize whole = this->size();
    int left = whole.width() - image_.width();
    left = left/2;
    int top = whole.height() - image_.height();
    top = top/2;
    painter.drawPixmap(left, top, image_);
}

void PanoramaWidget::resizeEvent(QResizeEvent * event)
{
    preferSize_ = event->size();
    emit refresh();
}
