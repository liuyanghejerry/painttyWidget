#include "pencil.h"

#include <QPen>
#include <QPainter>
#include <QDebug>

#include "../misc/singleton.h"
#include "../misc/shortcutmanager.h"

Pencil::Pencil()
{
    name_ = "Pencil";
    displayName_ = QObject::tr("Pencil");
    shortcut_ = Singleton<ShortcutManager>::instance()
            .shortcut("pencil")["key"].toString();
    pencil.setWidth(1);
    pencil.setCapStyle(Qt::RoundCap);
    pencil.setJoinStyle(Qt::RoundJoin);

    icon_.addFile("iconset/ui/pencil-1.png",
                  QSize(), QIcon::Disabled);
    icon_.addFile("iconset/ui/pencil-2.png",
                  QSize(), QIcon::Active);
    icon_.addFile("iconset/ui/pencil-3.png",
                  QSize(), QIcon::Selected);
    icon_.addFile("iconset/ui/pencil-3.png",
                  QSize(), QIcon::Normal, QIcon::On);
    icon_.addFile("iconset/ui/pencil-4.png",
                  QSize(), QIcon::Normal);
    updateCursor(this->width());
}

Pencil::~Pencil()
{
}

void Pencil::drawPoint(const QPointF &st)
{
    QPainter painter;
    if(!painter.begin(surface_->imagePtr())){
        return;
    }
    painter.setPen(pencil);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPoint(st);
    painter.end();
}

void Pencil::drawLine(const QPointF &st,
                      const QPointF &end,
                      qreal &)
{
    QPainter painter;
    if(!painter.begin(surface_->imagePtr())){
        return;
    }
    painter.setPen(pencil);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(st, end);
    painter.end();
}

void Pencil::setColor(const QColor &color)
{
    QColor c = color;
    c.setAlphaF(this->hardness()/100.0);
    pencil.setColor(c);
    mainColor = color;
}

int Pencil::width()
{
    return pencil.width();
}

void Pencil::setWidth(int w)
{
    pencil.setWidth(w);
    updateCursor(w);
}

void Pencil::setHardness(int h)
{
    hardness_ = h;
    QColor c = mainColor;
    c.setAlphaF(this->hardness()/100.0);
    pencil.setColor(c);
}

QVariantMap Pencil::brushInfo()
{
    QVariantMap map;
    QVariantMap colorMap;
    colorMap.insert("red", pencil.color().red());
    colorMap.insert("green", pencil.color().green());
    colorMap.insert("blue", pencil.color().blue());
    map.insert("width", QVariant(pencil.width()));
    map.insert("hardness", QVariant(this->hardness()));
    map.insert("color", colorMap);
    map.insert("name", this->name());
    return map;
}

QVariantMap Pencil::defaultInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(1));
    map.insert("hardness", QVariant(100));
    QVariantMap colorMap;
    colorMap.insert("red", 0);
    colorMap.insert("green", 0);
    colorMap.insert("blue", 0);
    map.insert("color", colorMap);
    return map;
}

AbstractBrush* Pencil::createBrush()
{
    return new Pencil;
}
