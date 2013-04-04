#ifndef ABSTRACTBRUSH_H
#define ABSTRACTBRUSH_H

#include <QKeySequence>
#include <QCursor>
#include <QPoint>
#include <QPainter>
#include <QImage>
#include <QIcon>
#include <QVariantMap>

#include "../misc/layer.h"

class AbstractBrush
{
public:
    AbstractBrush();
    virtual ~AbstractBrush();
    virtual QString name();
    virtual QString displayName();
    virtual QIcon icon();
    virtual QCursor cursor();
    virtual QKeySequence shortcut();
    virtual int width()=0;
    virtual void setWidth(int w)=0;
    virtual int hardness();
    virtual void setHardness(int h);
    virtual QColor color();
    virtual void setColor(const QColor &color);

    virtual void start(const QPointF &st);
    virtual void lineTo(const QPointF &st);
    virtual void end();

    virtual QVariantMap defaultInfo()=0;
    virtual QVariantMap brushInfo()=0;
    virtual QPointF lastPoint();
    virtual void setLastPoint(const QPointF &p);
    virtual LayerPointer surface();
    virtual void setSurface(LayerPointer p);
    virtual AbstractBrush* createBrush()=0;

protected:
    QString name_;
    QString displayName_;
    QIcon icon_;
    QCursor cursor_;
    QKeySequence shortcut_;
    LayerPointer surface_;
    QPointF lastPoint_;
    QPointF startPoint_;
    qreal leftOverDistance;
    QColor mainColor;
    int hardness_;
    virtual void drawPoint(const QPointF &st);
    virtual void drawLine(const QPointF &,
                          const QPointF &,
                          qreal &);
    virtual void updateCursor(int w);
};

#endif // ABSTRACTBRUSH_H
