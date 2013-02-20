#include "pencil.h"

Pencil::Pencil(QObject *parent) :
    Brush(parent)
{
    pencil.setCapStyle(Qt::RoundCap);
    pencil.setJoinStyle(Qt::RoundJoin);
}

void Pencil::drawPoint(const QPointF &st)
{
    QPainter painter;
    if(directDraw_ && surface_ ){
        painter.begin(surface_);
    }else{
        painter.begin(&result);
    }
    painter.setPen(pencil);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPoint(st);
}

void Pencil::drawLine(const QPointF &st,
                      const QPointF &end,
                      qreal &left)
{
    QPainter painter;
    if(directDraw_ && surface_ ){
        painter.begin(surface_);
    }else{
        painter.begin(&result);
    }
    painter.setPen(pencil);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(st, end);
}

bool Pencil::loadStencil(const QString &)
{
    return true;
}

void Pencil::setColor(const QColor &color)
{
    pencil.setColor(color);
    mainColor = color;
}

int Pencil::width()
{
    return pencil.width();
}

void Pencil::setWidth(int w)
{
    pencil.setWidth(w);
}

QVariantMap Pencil::brushInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(pencil.width()));
    map.insert("color", QVariant(pencil.color()));
    map.insert("name", QVariant("Pencil"));
    return map;
}

QVariantMap Pencil::defaultInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(1));
    map.insert("color", QVariant(QColor(Qt::black).toHsv()));
    return map;
}
