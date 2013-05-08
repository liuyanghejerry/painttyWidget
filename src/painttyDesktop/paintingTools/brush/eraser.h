#ifndef ERASER_H
#define ERASER_H

#include "abstractbrush.h"

class Eraser : public AbstractBrush
{
public:
    Eraser();
    ~Eraser();
    void setColor(const QColor &);
    int width();
    void setWidth(int w);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
    AbstractBrush* createBrush();

protected:
    void drawPoint(const QPointF &st, qreal pressure=1.0);
    void drawLine(const QPointF &st, const QPointF &end, qreal &left, qreal pressure=1.0);
private:
    int width_;
};

#endif // ERASER_H
