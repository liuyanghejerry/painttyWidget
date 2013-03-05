#ifndef SKETCHBRUSH_H
#define SKETCHBRUSH_H

#include <QVector>
#include "brush.h"

class SketchBrush : public Brush
{
    Q_OBJECT
public:
    explicit SketchBrush(QObject *parent = 0);
    void start(const QPointF &st);
    bool loadStencil(const QString &);
    void setColor(const QColor &color);
    int width();
    void setWidth(int w);
    void setLastPoint(const QPointF &p);
    void lineTo(const QPointF &st);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
    
signals:
    
public slots:
    
protected:
    QVector<QPointF> points;
    QPixmap thinStencil;
    QPen sketchPen;
    void sketch();
    void preparePen();
};

#endif // SKETCHBRUSH_H
