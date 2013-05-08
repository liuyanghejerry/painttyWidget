#ifndef SKETCHBRUSH_H
#define SKETCHBRUSH_H

#include <QList>
#include "abstractbrush.h"

class SketchBrush : public AbstractBrush
{
public:
    SketchBrush();
    ~SketchBrush();
    void start(const QPointF &st, qreal pressure=1.0);
    void setColor(const QColor &color);
    int width();
    void setWidth(int w);
    void setLastPoint(const QPointF &p);
    void lineTo(const QPointF &st, qreal pressure=1.0);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
    AbstractBrush* createBrush();
    
protected:
    QList<QPointF> points;
    QPen sketchPen;
    void sketch();
    void preparePen();
};

#endif // SKETCHBRUSH_H
