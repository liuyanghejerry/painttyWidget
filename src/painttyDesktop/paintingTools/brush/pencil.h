#ifndef PENCIL_H
#define PENCIL_H

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
    void setHardness(int h);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
    AbstractBrush* createBrush();
protected:
    void drawPoint(const QPointF &st, qreal pressure=1.0);
    void drawLine(const QPointF &st,
                  const QPointF &end,
                  qreal &left,
                  qreal pressure=1.0);

protected:
    QPen pencil;
};

#endif // PENCIL_H
