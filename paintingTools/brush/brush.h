#ifndef BRUSH_H
#define BRUSH_H

#include <QObject>
#include <QPoint>
#include <QPainter>
#include <QFile>
#include <QImage>
#include <QVariantMap>
#include <QtCore/qmath.h>

class Brush : public QObject
{
    Q_OBJECT
public:
    explicit Brush(QObject *parent = 0);
    ~Brush();
    virtual bool loadStencil(const QString &fileName);
    void setSize(const QSize &s);
    virtual int width();
    virtual void setWidth(int w);
    void setDirectDraw(bool enable);
    bool isDirectDraw();
    QPixmap * surface();
    void setSurface(QPixmap *p);
    virtual QColor color();
    virtual void setColor(const QColor &color);
    virtual void start(const QPointF &st);
    virtual void lineTo(const QPointF &st);
    virtual void end();
    virtual QVariantMap defaultInfo();

    virtual QVariantMap brushInfo();
    QPixmap pixmap();

signals:
    
public slots:
protected:
    QPixmap *surface_;
    QPixmap result;
    QPointF lastPoint_;
    QPointF startPoint_;
    QFile stencilFile;
    QPixmap stencil;
    QImage originalStencil;
    uchar *brushData;
    QColor mainColor;
    qreal leftOverDistance;
    bool directDraw_;
    virtual void effect();
    virtual void drawPoint(const QPointF &st);
    virtual void drawLine(const QPointF &st, const QPointF &end, qreal &left);
    void clear();

};

#endif // BRUSH_H
