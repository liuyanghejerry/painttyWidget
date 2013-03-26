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
signals:
    
public slots:
protected:
    void drawPoint(const QPointF &st);
    void drawLine(const QPointF &st, const QPointF &end, qreal &left);
private:
    int width_;
};

#endif // ERASER_H
