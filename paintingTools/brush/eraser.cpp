#include "eraser.h"

Eraser::Eraser(QObject *parent) :
    Brush(parent),
    width_(100)
{
    mainColor = Qt::transparent;
}

bool Eraser::loadStencil(const QString &)
{
    return true;
}

void Eraser::setColor(const QColor &)
{
    return;
}

int Eraser::width()
{
    return width_;
}

void Eraser::setWidth(int w)
{
    width_ = w;
}

void Eraser::drawPoint(const QPointF &st)
{
    QPainter painter(surface_);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen eraser(mainColor);
    eraser.setWidth(width_);
    eraser.setCapStyle(Qt::RoundCap);
    eraser.setJoinStyle(Qt::RoundJoin);
    painter.setPen(eraser);
    painter.setBrush(mainColor);
    painter.drawPoint(st.x(), st.y());
}

void Eraser::drawLine(const QPointF &st, const QPointF &end, qreal &left)
{
    QPainter painter(surface_);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen eraser(mainColor);
    eraser.setWidth(width_);
    eraser.setCapStyle(Qt::RoundCap);
    eraser.setJoinStyle(Qt::RoundJoin);
    painter.setPen(eraser);
    painter.setBrush(mainColor);
    painter.drawLine(st, end);
}

QVariantMap Eraser::brushInfo()
{
    QVariantMap map;
    QVariantMap colorMap;
    colorMap.insert("red", this->color().red());
    colorMap.insert("green", this->color().green());
    colorMap.insert("blue", this->color().blue());
    map.insert("width", QVariant(this->width()));
    map.insert("color", colorMap);
    map.insert("name", QVariant("Eraser"));
    return map;
}

QVariantMap Eraser::defaultInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(10));
    QVariantMap colorMap;
    colorMap.insert("red", 255);
    colorMap.insert("green", 255);
    colorMap.insert("blue", 255);
    map.insert("color", colorMap);
    return map;
}
