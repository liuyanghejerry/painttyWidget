#include "basicbrush.h"
#include <QRadialGradient>
#include <QPainter>
#include <QEasingCurve>
#include <QPen>
#include <QBrush>
#include <QtCore/qmath.h>
#include <QtGlobal>
#include <QDebug>

#include "../../misc/shortcutmanager.h"
#include "../../misc/singleton.h"

//qreal myEasingFunction(qreal progress);

BasicBrush::BasicBrush() :
    AbstractBrush(),
    left_(0)
{
    typedef BrushFeature BF;
    BF::FeatureBits bits;
    bits.set(BF::WIDTH);
    bits.set(BF::COLOR);
    bits.set(BF::WATER);
    bits.set(BF::THICKNESS);
    features_ = bits;

    name_ = "BasicBrush";
    displayName_ = QObject::tr("BasicBrush");
    shortcut_ = Singleton<ShortcutManager>::instance()
            .shortcut("basicbrush")["key"].toString();
}

void BasicBrush::setWidth(int width)
{
    AbstractBrush::setWidth(width);
    makeStencil(color_);
}

void BasicBrush::setColor(const QColor &color)
{
    AbstractBrush::setColor(color);
    makeStencil(color_);
}

void BasicBrush::setThickness(int thickness)
{
    AbstractBrush::setThickness(thickness);
    makeStencil(color_);
}

void BasicBrush::makeStencil(QColor color)
{
    if(stencil_.isNull() || stencil_.width() != width_){
        stencil_ = QImage(width_, width_, QImage::Format_ARGB32_Premultiplied);
    }
    stencil_.fill(Qt::transparent);
    const QEasingCurve easing(QEasingCurve::OutQuart);

    const int half_width = width_>>1;

    const QPoint center(half_width, half_width);
    QPainter painter;
    if(!painter.begin(&stencil_)){
        return;
    }
    QRadialGradient gradient(center, half_width);
    const qreal thk = thickness_/500.0;
    for(int i=0;i<100;++i){
        const qreal value = i/100.0;
//        qreal thk = 0.03;
        color.setAlphaF((1-easing.valueForProgress(value))*thk);
        gradient.setColorAt(value, color);
    }
    gradient.setColorAt(1, Qt::transparent);
    gradient.setCenterRadius(half_width);
    const qreal r_h = thickness_*0.007; // make it soft to fake an Anti-aliasing effect
//    qreal r_h = 0.8;
    gradient.setFocalRadius(half_width*r_h -1);
    const QBrush brush(gradient);
    const QPen pen(brush, width_);
    painter.setPen(pen);
    painter.drawPoint(half_width, half_width);
    painter.end();
}

void BasicBrush::drawPointInternal(const QPoint &p,
                                   const QImage& stencil,
                                   QPainter* painter)
{
    // TODO: add pressure
    bool need_delete = false;
    if(!painter) {
        painter = new QPainter;
        need_delete = true;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->begin(surface_->imagePtr());
    }
    painter->drawImage(p.x(), p.y(), stencil);
    if(need_delete) {
        painter->end();
        delete painter;
    }
}

void BasicBrush::drawPoint(const QPoint &p, qreal pr)
{
    qDebug()<<pr;
    QPainter painter(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    QImage pressure_stencil = stencil_.scaledToWidth(stencil_.width()*pr);
    drawPointInternal(QPoint(p.x() - (pressure_stencil.width()>>1),
                             p.y() - (pressure_stencil.height()>>1)),
                      pressure_stencil,
                      &painter);
    last_point_ = p;
}

void BasicBrush::drawLineTo(const QPoint &end, qreal pressure)
{
    qDebug()<<pressure;
    if(end.x() > surface_->imageConstPtr()->width() || end.x() < 0
            || end.y() > surface_->imageConstPtr()->height() || end.y() < 0) {
        return;
    }
    const QPoint& start = last_point_;
    const qreal spacing = width_*pressure*0.1;

    const qreal deltaX = end.x() - start.x();
    const qreal deltaY = end.y() - start.y();

    const qreal distance = qSqrt( deltaX * deltaX + deltaY * deltaY );
    qreal stepX = 0.0;
    qreal stepY = 0.0;
    if ( distance > 0.0 ) {
        qreal invertDistance = 1.0 / distance;
        stepX = deltaX * invertDistance;
        stepY = deltaY * invertDistance;
    }

    qreal offsetX = 0.0;
    qreal offsetY = 0.0;

    qreal totalDistance = left_ + distance;
    // TODO
    QImage pressure_stencil = stencil_.scaledToWidth(stencil_.width()*pressure);

    QPainter painter(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    while ( totalDistance >= spacing ) {
        if ( left_ > 0.0 ) {
            offsetX += stepX * (spacing - left_);
            offsetY += stepY * (spacing - left_);
            drawPointInternal(QPoint(start.x() + offsetX - (pressure_stencil.width()>>1),
                                     start.y() + offsetY - (pressure_stencil.height()>>1)),
                              pressure_stencil,
                              &painter);
            left_ -= spacing;
        } else {
            offsetX += stepX * spacing;
            offsetY += stepY * spacing;
            drawPointInternal(QPoint(start.x() + offsetX - (pressure_stencil.width()>>1),
                                     start.y() + offsetY - (pressure_stencil.height()>>1)),
                              pressure_stencil,
                              &painter);
        }
        totalDistance -= spacing;
    }
    left_ = totalDistance;
    last_point_ = end;
}

AbstractBrush *BasicBrush::createBrush()
{
    return new BasicBrush;
}
