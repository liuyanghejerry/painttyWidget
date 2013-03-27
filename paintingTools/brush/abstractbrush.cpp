#include "abstractbrush.h"

AbstractBrush::AbstractBrush()
{
    //
}

AbstractBrush::~AbstractBrush()
{
    //
}


QIcon AbstractBrush::icon()
{
    return icon_;
}

QCursor AbstractBrush::cursor()
{
    return cursor_;
}

QKeySequence AbstractBrush::shortcut()
{
    return shortcut_;
}

void AbstractBrush::start(const QPointF &st)
{
    leftOverDistance = 0;
    drawPoint(st);
    lastPoint_ = st;
}

void AbstractBrush::lineTo(const QPointF &st)
{
    if(lastPoint_.isNull()){
        start(st);
        return;
    }
    this->drawLine(lastPoint_, st, leftOverDistance);
    lastPoint_ = st;
}

QColor AbstractBrush::color()
{
    return mainColor;
}

void AbstractBrush::setColor(const QColor &c)
{
    mainColor = c;
}

int AbstractBrush::hardness()
{
    return hardness_;
}

void AbstractBrush::setHardness(int h)
{
    hardness_ = h;
}

LayerPointer AbstractBrush::surface()
{
    return surface_;
}

void AbstractBrush::setSurface(LayerPointer p)
{
    surface_ = p;
}

void AbstractBrush::end()
{
}

void AbstractBrush::drawPoint(const QPointF &)
{
    //
}

void AbstractBrush::drawLine(const QPointF &,
                      const QPointF &,
                      qreal &)
{
    //
}

QPointF AbstractBrush::lastPoint()
{
    return lastPoint_;
}


void AbstractBrush::setLastPoint(const QPointF &p)
{
    lastPoint_ = p;
}
