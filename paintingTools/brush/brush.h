#ifndef BRUSH_H
#define BRUSH_H

#include "abstractbrush.h"

class Brush : public AbstractBrush
{
public:
    Brush();
    ~Brush();
    QIcon icon();
    QCursor cursor();
    QKeySequence shortcut();
    int width();
    void setWidth(int w);
    void setHardness(int h);
    int hardness();
    QColor color();
    void setColor(const QColor &color);
    void start(const QPointF &st);
    void lineTo(const QPointF &st);
    QVariantMap defaultInfo();
    QVariantMap brushInfo();

protected:
    QFile stencilFile;
    QPixmap stencil;
    QImage originalStencil;
    uchar *brushData;
    QColor mainColor;
    int hardness_;
    void drawPoint(const QPointF &st);
    void drawLine(const QPointF &st, const QPointF &end, qreal &left);
    bool loadStencil(const QString &fileName);
};

#endif // BRUSH_H
