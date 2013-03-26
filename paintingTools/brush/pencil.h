#ifndef PENCIL_H
#define PENCIL_H

#include <QPen>
#include <QPainter>
#include <QVariant>
#include "abstractbrush.h"

class Pencil : public AbstractBrush
{
public:
    Pencil();
    ~Pencil();
    void setColor(const QColor &color);
    int width();
    void setWidth(int w);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
protected:
    void drawPoint(const QPointF &st);
    void drawLine(const QPointF &st,
                  const QPointF &end,
                  qreal &left);

protected:
    QPen pencil;
};

#endif // PENCIL_H
