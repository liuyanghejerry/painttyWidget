#include "sketchbrush.h"

SketchBrush::SketchBrush()
{
    name_ = "Sketch";
    displayName_ = QObject::tr("Sketch");
    shortcut_ = Qt::Key_S;
    icon_.addFile("iconset/ui/sketch-1.png",
                  QSize(), QIcon::Disabled);
    icon_.addFile("iconset/ui/sketch-2.png",
                  QSize(), QIcon::Active);
    icon_.addFile("iconset/ui/sketch-3.png",
                  QSize(), QIcon::Selected);
    icon_.addFile("iconset/ui/sketch-3.png",
                  QSize(), QIcon::Normal, QIcon::On);
    icon_.addFile("iconset/ui/sketch-4.png",
                  QSize(), QIcon::Normal);
    updateCursor(this->width());
}

SketchBrush::~SketchBrush()
{

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
    updateCursor(w);
}

void SketchBrush::start(const QPointF &st)
{
    preparePen();
    leftOverDistance = 0;
    points.clear();
    points.push_back(st);
    lastPoint_ = st;
}

void SketchBrush::preparePen()
{
    QColor subColor = mainColor;
    subColor.setAlphaF(this->hardness()/100.0);
    sketchPen.setColor(subColor);
    sketchPen.setCapStyle(Qt::RoundCap);
    sketchPen.setJoinStyle(Qt::RoundJoin);
}

void SketchBrush::lineTo(const QPointF &st)
{
    if(lastPoint_.isNull()){
        start(st);
        return;
    }
    points.push_back(st);
    sketch();
    lastPoint_ = st;
}

void SketchBrush::setLastPoint(const QPointF &p)
{
    lastPoint_ = p;
    points.clear();
}

void SketchBrush::sketch()
{
    if(points.count() > 10){
        QPainterPath path;
        path.moveTo(points[0]);
        for(int i=0;i<points.count()-10;i+=4){
            path.cubicTo(points[i], points[i+2], points[i+9]);
        }
        points.pop_front();

        QPainter painter;
        if(!painter.begin(surface_->imagePtr())){
            return;
        }
        painter.setRenderHint(QPainter::Antialiasing);
        painter.strokePath(path, sketchPen);
        painter.end();
    }
}

QVariantMap SketchBrush::brushInfo()
{
    QVariantMap map;
    QVariantMap colorMap;
    colorMap.insert("red", sketchPen.color().red());
    colorMap.insert("green", sketchPen.color().green());
    colorMap.insert("blue", sketchPen.color().blue());
    map.insert("width", sketchPen.width());
    map.insert("hardness", this->hardness());
    map.insert("color", colorMap);
    map.insert("name", this->name());
    return map;
}

QVariantMap SketchBrush::defaultInfo()
{
    QVariantMap map;
    QVariantMap colorMap;
    colorMap.insert("red", 160);
    colorMap.insert("green", 160);
    colorMap.insert("blue", 164);
    map.insert("width", 1);
    map.insert("hardness", 7);
    map.insert("color", colorMap);
    return map;
}

AbstractBrush* SketchBrush::createBrush()
{
    return new SketchBrush;
}
