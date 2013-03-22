#ifndef SIMPLEPEN_H
#define SIMPLEPEN_H

#include "basicbrush.h"
#include <QPainter>

class SimplePen : public BasicBrush
{
    Q_OBJECT

public:
    SimplePen(QObject *parent = 0);
    QString brushIdentifer() const;
    QString brushName() const;
    QIcon brushIcon() const;
    QKeySequence brushShotcut() const;
    QCursor brushCursor() const;
    int brushWidth() const;
    void setBrushWidth(int width);
    QColor brushColor() const;
    void setBrushColor(const QColor &color);
    int brushSolidity() const;
    void setBrushSolidity(int solidity);
    QVariantMap brushInfo() const;
    void setBrushInfo(const QVariantMap& brushInfo);
    QPixmap* surface() const;
    void setSurface(QPixmap *surface);
    void start(const QPointF &startPoint, qreal pressure = 1);
    void moveTo(const QPointF &newPoint, qreal pressure = 1);
    void end();

private:
    QPainter painter;
    int _brushWidth;
    QColor _brushColor;
    QPixmap *_surface;
    QPointF _lastPoint;
    
};

#endif // SIMPLEPEN_H
