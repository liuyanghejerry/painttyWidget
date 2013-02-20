#include "sketchbrush.h"

SketchBrush::SketchBrush(QObject *parent) :
    Brush(parent)
{

}

bool SketchBrush::loadStencil(const QString &)
{
    return true;
}

void SketchBrush::setColor(const QColor &color)
{
    mainColor = color;
}

int SketchBrush::width()
{
    return sketchPen.width();
}

void SketchBrush::setWidth(int w)
{
    sketchPen.setWidth(w);
}

void SketchBrush::start(const QPointF &st)
{
    clear();
    preparePen();
    leftOverDistance = 0;
    points.clear();
    points.push_back(st);
    lastPoint_ = st;
}

void SketchBrush::preparePen()
{
    //    sketchPen.setWidthF(10);
    QColor subColor = mainColor;
    subColor.setAlphaF(0.07);
    sketchPen.setColor(subColor);
    sketchPen.setCapStyle(Qt::RoundCap);
    sketchPen.setJoinStyle(Qt::RoundJoin);
}

void SketchBrush::lineTo(const QPointF &st)
{
    clear();
    points.push_back(st);
    sketch();
    lastPoint_ = st;
}

void SketchBrush::sketch()
{
    if(points.count() > 10){
        QPainterPath path;
        path.moveTo(points[0]);
        for(int i=0;i<points.count()-10;i+=4){
            //            path.quadTo(points[i], points[i+6]);
            path.cubicTo(points[i], points[i+2], points[i+9]);

        }
        points.pop_front();

        QPainter painter;
        if(directDraw_ && surface_){
            painter.begin(surface_);
        }else{
            painter.begin(&result);
        }
        painter.setRenderHint(QPainter::Antialiasing);
        painter.strokePath(path, sketchPen);
    }
}

QVariantMap SketchBrush::brushInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(sketchPen.width()));
    map.insert("color", QVariant(sketchPen.color()));
    map.insert("name", QVariant("Sketch"));
    return map;
}

QVariantMap SketchBrush::defaultInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(1));
    map.insert("color", QVariant(QColor(Qt::gray).toHsv()));
    return map;
}
