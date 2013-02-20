#ifndef ERASER_H
#define ERASER_H

#include "brush.h"

class Eraser : public Brush
{
    Q_OBJECT
public:
    explicit Eraser(QObject *parent = 0);
    virtual bool loadStencil(const QString &);
    virtual void setColor(const QColor &);
    virtual int width();
    virtual void setWidth(int w);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
signals:
    
public slots:
protected:
    virtual void drawPoint(const QPointF &st);
    virtual void drawLine(const QPointF &st, const QPointF &end, qreal &left);
private:
    int width_;
};

#endif // ERASER_H
