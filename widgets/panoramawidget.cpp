#include "panoramawidget.h"

PanoramaWidget::PanoramaWidget(QWidget *parent) :
    QWidget(parent),
    preferSize_(144, 96),
    image_( preferSize_ )
{
    QPalette p = this->palette();
    p.setColor(QPalette::Background, Qt::gray);
    this->setPalette(p);
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

void PanoramaWidget::onImageChange(QPixmap p,
                                   const QRect &r)
{
    QPainter painter(&p);
    QPen pen;
    pen.setColor(this->palette().color(QPalette::Text));
    pen.setWidth(30);
    painter.setPen(pen);
    painter.drawRect(r);
    image_ = p.scaledToWidth(preferSize_.width());
//    if(p.width() >= p.height()){
//        int delta = p.width()/preferSize_.width();

//    }else{
//        //
//    }


    update();
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
