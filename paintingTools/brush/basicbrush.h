#ifndef BASICBRUSH_H
#define BASICBRUSH_H

#include <QObject>
#include <QCursor>

class BasicBrush : public QObject
{
    Q_OBJECT
public:
    BasicBrush(QObject *parent = 0) : QObject(parent) {}
    virtual ~BasicBrush() = 0;
    virtual QString brushIdentifer() const = 0;
    virtual QIcon brushIcon() const = 0;
    virtual QKeySequence brushShotcut() const = 0;
    virtual QCursor brushCursor() const = 0;
    virtual int brushWidth() const = 0;
    virtual void setBrushWidth(int width) = 0;
    virtual QColor brushColor() const = 0;
    virtual void setBrushColor(const QColor &color) = 0;
    virtual qreal brushPressure() const = 0;
    virtual void setBrushPressure(qreal pressure) = 0;
    virtual QPixmap* surface() const = 0;
    virtual void setSurface(QPixmap *surface) = 0;
    virtual void start(const QPointF &startPoint) = 0;
    virtual void moveTo(const QPointF &newPoint) = 0;
    virtual void end() = 0;
    
signals:
    void newBrushCursor(QCursor);
    
public slots:
    
};

#endif // BASICBRUSH_H
