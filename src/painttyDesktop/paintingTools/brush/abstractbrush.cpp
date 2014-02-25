#include "abstractbrush.h"
#include <QDebug>
#include <QPainter>

typedef BrushFeature::LIMIT BFL;

AbstractBrush::AbstractBrush():
    width_(10),
    thickness_(BFL::THICKNESS_MAX),
    color_(Qt::black),
    surface_(nullptr)
{
    typedef BrushFeature BF;
    BF::FeatureBits bits;
    bits.set(BF::WIDTH);
    bits.set(BF::COLOR);
    bits.set(BF::THICKNESS);
    features_ = bits;
}

AbstractBrush::~AbstractBrush()
{
    //
}

QString AbstractBrush::name()
{
    return name_;
}

QString AbstractBrush::displayName()
{
    return displayName_;
}

QIcon AbstractBrush::icon()
{
    return icon_;
}

QCursor AbstractBrush::cursor()
{
    return cursor_;
}

QKeySequence AbstractBrush::shortcut()
{
    return shortcut_;
}

void AbstractBrush::setShortcut(QKeySequence key)
{
    shortcut_ = key;
}

int AbstractBrush::width() const
{
    return width_;
}

void AbstractBrush::setWidth(int width)
{
    width_ = qBound<int>(BFL::WIDTH_MIN, width, BFL::WIDTH_MAX);
    settings_.insert("width", width_);
    updateCursor(width);
}
int AbstractBrush::thickness() const
{
    return thickness_;
}

void AbstractBrush::setThickness(int thickness)
{
    thickness_ = qBound<int>(BFL::THICKNESS_MIN, thickness, BFL::THICKNESS_MAX);
    settings_.insert("thickness", thickness_);
}

Surface AbstractBrush::surface() const
{
    return surface_;
}

void AbstractBrush::setSurface(Surface surface)
{
    surface_ = surface;
}

bool AbstractBrush::support(const BrushFeature::FEATURE &f)
{
    return features_.support(f);
}

BrushFeature AbstractBrush::features()
{
    return features_;
}

BrushSettings AbstractBrush::settings() const
{
    return settings_;
}

void AbstractBrush::setSettings(const BrushSettings &settings)
{
    const BrushSettings& s = settings;
    QVariantMap colorMap = settings["color"].toMap();
    QColor color(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());
    setColor(color);
    setWidth(s.value("width", width_).toInt());
    setThickness(s.value("thickness", thickness_).toInt());
    settings_ = s;
}

BrushSettings AbstractBrush::defaultSettings() const
{
    BrushSettings s;
    s.insert("width", 10);
    s.insert("thickness", BFL::THICKNESS_MAX);
    QVariantMap color;
    color.insert("red", 0);
    color.insert("green", 0);
    color.insert("blue", 0);
    s.insert("color", color);
    return s;
}

void AbstractBrush::updateCursor(int w)
{
    int frame = w+2+w%2; // +2 for a border padding
    QPixmap img(frame, frame);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.drawEllipse(0, 0, w, w);
    const int half_frame = frame>>1;
    if (w > 10)
        painter.drawPoint(half_frame, half_frame);
    cursor_ = QCursor(img, half_frame, half_frame);
}

QColor AbstractBrush::color() const
{
    return color_;
}

void AbstractBrush::setColor(const QColor &color)
{
    color_ = color;
    QVariantMap colorMap;
    colorMap.insert("red", color_.red());
    colorMap.insert("green", color_.green());
    colorMap.insert("blue", color_.blue());
    settings_.insert("color", colorMap);
}
