#ifndef ABSTRACTBRUSH_H
#define ABSTRACTBRUSH_H

#include <QPoint>
#include <QImage>
#include <QIcon>
#include <QCursor>
#include <QKeySequence>
#include <QColor>
#include <QSharedPointer>
#include "brushfeature.h"
#include "brushsettings.h"
#include "../misc/layer.h"
typedef LayerPointer Surface;

class AbstractBrush
{
public:
    explicit AbstractBrush();
    // we need a virtual destructor, since it is deleted via base class
    // or we'll have risk of memory leak when sub-class has its own heap data.
    virtual ~AbstractBrush();

    virtual QString name();
    virtual QString displayName();
    virtual QIcon icon();
    virtual QCursor cursor();
    virtual QKeySequence shortcut();
    virtual void setShortcut(QKeySequence key);

    virtual int width() const;
    virtual void setWidth(int width);
    virtual int thickness() const;
    virtual void setThickness(int thickness);
    virtual QColor color() const;
    virtual void setColor(const QColor &color);

    virtual Surface surface() const;
    // Brush does NOT take ownership of surface
    virtual void setSurface(Surface surface);

    virtual bool support(const BrushFeature::FEATURE &f);
    virtual BrushFeature features();

    virtual void drawPoint(const QPoint& p, qreal pressure=1)=0;
    virtual void drawLineTo(const QPoint& end, qreal pressure=1)=0;

    virtual BrushSettings settings() const;
    virtual void setSettings(const BrushSettings &settings);
    virtual BrushSettings defaultSettings() const;
    virtual AbstractBrush* createBrush()=0;

protected:
    int width_;
    int thickness_;
    QColor color_;
    Surface surface_;
    QImage stencil_;
    QPoint last_point_;
    BrushSettings settings_;
    BrushFeature features_;

    QString name_;
    QString displayName_;
    QIcon icon_;
    QCursor cursor_;
    QKeySequence shortcut_;

    virtual void updateCursor(int w);
};

#endif // ABSTRACTBRUSH_H
