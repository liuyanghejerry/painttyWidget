#ifndef BASICBRUSH_H
#define BASICBRUSH_H

#include "abstractbrush.h"
#include <QImage>

class BasicBrush : public AbstractBrush
{
public:
    enum: int {
        HARDNESS_MAX = 100,
        HARDNESS_MIN = 0
    };
    explicit BasicBrush();
    void setWidth(int width) Q_DECL_OVERRIDE;
    void setColor(const QColor &color) Q_DECL_OVERRIDE;
    void setThickness(int thickness) Q_DECL_OVERRIDE;

    void drawPoint(const QPoint& p, qreal pressure=1) Q_DECL_OVERRIDE;
    void drawLineTo(const QPoint& end, qreal pressure=1) Q_DECL_OVERRIDE;

    AbstractBrush* createBrush() Q_DECL_OVERRIDE;

    int hardness() const;
    void setHardness(int hardness);

signals:

public slots:

protected:
    qreal left_;
    int hardness_;
    virtual void makeStencil(QColor color);
    virtual void drawPointInternal(const QPoint& p, const QImage &stencil, QPainter *painter);
};

#endif // BASICBRUSH_H
