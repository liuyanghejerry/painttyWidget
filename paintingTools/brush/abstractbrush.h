#ifndef ABSTRACTBRUSH_H
#define ABSTRACTBRUSH_H

#include <QObject>
#include <QKeySequence>
#include <QCursor>
#include <QPoint>
#include <QPainter>
#include <QFile>
#include <QImage>
#include <QIcon>
#include <QVariantMap>
#include <QtCore/qmath.h>

class AbstractBrush
{
public:
    AbstractBrush();
    virtual ~AbstractBrush();
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
    virtual QPixmap * surface();
    virtual void setSurface(QPixmap *p);

protected:
    QIcon icon_;
    QCursor cursor_;
    QKeySequence shortcut_;
    QPixmap *surface_;
    QPointF lastPoint_;
    QPointF startPoint_;
    qreal leftOverDistance;
    QColor mainColor;
    int hardness_;
    virtual void drawPoint(const QPointF &st);
    virtual void drawLine(const QPointF &st,
                          const QPointF &end,
                          qreal &left);
};

#endif // ABSTRACTBRUSH_H
