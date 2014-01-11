#include "waterbased.h"
#include <QtGlobal>
#include <functional>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QBitmap>
#include <QObject>
#include <QDebug>

WaterBased::WaterBased():
    BasicBrush(),
    water_(50),
    extend_(50),
    mixin_(20),
    color_remain_(255)
{
    typedef BrushFeature BF;
    BF::FeatureBits bits;
    bits.set(BF::WIDTH);
    bits.set(BF::COLOR);
    bits.set(BF::THICKNESS);
    bits.set(BF::WATER);
    bits.set(BF::EXTEND);
    bits.set(BF::MIXIN);
    features_ = bits;

    name_ = QObject::tr("WaterBrush");
    displayName_ = name_;
    shortcut_ = Qt::Key_3;
}
int WaterBased::water() const
{
    return water_;
}

void WaterBased::setWater(int water)
{
    water_ = qBound((int)WATER_MIN, water, (int)WATER_MAX);
}
int WaterBased::extend() const
{
    return extend_;
}

void WaterBased::setExtend(int extend)
{
    extend_ = qBound((int)EXTEND_MIN, extend, (int)EXTEND_MAX);
}
int WaterBased::mixin() const
{
    return mixin_;
}

void WaterBased::setMixin(int mixin)
{
    mixin_ = qBound((int)MIXIN_MIN, mixin, (int)MIXIN_MAX);
}

// non-reentrant.
inline QImage circle_mask(const int width)
{
    static QImage mask;
    if(mask.width() != width || mask.isNull()){
        mask = QImage(width, width, QImage::Format_ARGB32_Premultiplied);
        mask.fill(Qt::transparent);
        QPainter painter;
        // draw mask
        painter.begin(&mask);
        QPen pen(Qt::transparent);
        painter.setPen(pen);
        QBrush brush(Qt::black);
        painter.setBrush(brush);
        painter.drawEllipse(0, 0, width, width);
        painter.end();
    }
    return mask;
}

// non-reentrant.
inline QRgb avg_rgb(const QImage& square)
{
    const QRgb tran_rgb = QColor(Qt::transparent).rgba();
    QRgb target = tran_rgb;
    const QRgb* img_data = (const QRgb*) square.constBits();
    const size_t img_data_len = square.byteCount() >> (sizeof(QRgb)>>1);

    unsigned int r_sum = 0;
    unsigned int g_sum = 0;
    unsigned int b_sum = 0;

    size_t colored = 0;
    for(size_t i = 0;i<img_data_len;++i){
        QRgb next = *(img_data+i);
        if(tran_rgb == next){
            continue;
        }
        ++colored;
        // NOTE: *_sum won't overflow, unless we have a really giant square
        // we can also calculate avg one by one, but that's really lossy
        r_sum += qRed(next);
        g_sum += qGreen(next);
        b_sum += qBlue(next);
    }
    target = qRgb(r_sum / colored,
                  g_sum / colored,
                  b_sum / colored);
    return target;
}

QColor WaterBased::fetchColor(const QPoint& center) const
{
    const int delta_width = width_ >>1;
    const QPoint start_point = std::move(center - QPoint(delta_width, delta_width));

    QImage square = surface_->imagePtr()->copy(QRect(start_point, QSize(width_, width_)));

    QImage&& mask = circle_mask(square.width());
    QPainter painter;
    // composite
    painter.begin(&square);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawImage(0, 0, mask);
    painter.end();

    const QRgb c = avg_rgb(square);
    return QColor::fromRgb(c);
}

int WaterBased::mingleValue(int a, int b) const
{
    const qreal new_part = mixin_ / 100.0;
    const qreal old_part = 1 - new_part;

    return qBound(0.0,
                  (a*old_part) + (b*new_part),
                  255.0);
}

template<typename COLOR_FUNC>
int WaterBased::mingleSubColor(const QColor& color1,
                               const QColor& color2,
                               COLOR_FUNC f) const
{
    using namespace std;
    const auto color_fun = [&f](const QColor& color) -> int{
        const auto& func = bind(f, placeholders::_1);
        return func(color);
    };

    return mingleValue(color_fun(color1), color_fun(color2));
}

QColor WaterBased::mingleColor(const QColor &new_c)
{
    const int r = mingleSubColor
            <decltype(&QColor::red)>(mingled_color_, new_c, &QColor::red);
    const int g = mingleSubColor
            <decltype(&QColor::green)>(mingled_color_, new_c, &QColor::green);
    const int b = mingleSubColor
            <decltype(&QColor::blue)>(mingled_color_, new_c, &QColor::blue);

    color_remain_ -= (100 - water_)/1000.0;
    color_remain_ = qBound(0, color_remain_, 255);
    const int& a = color_remain_;

    return mingled_color_ = QColor::fromRgba(qRgba(r, g, b, a));
}

void WaterBased::drawPoint(const QPoint &p, qreal )
{
    last_color_ = mingled_color_ = fetchColor(p);
    last_point_ = p;
    color_remain_ = 255;
}

void WaterBased::drawLineTo(const QPoint &end, qreal presure)
{
    last_color_ = fetchColor(last_point_);
    makeStencil(mingleColor(last_color_));
    BasicBrush::drawLineTo(end, presure);
}

void WaterBased::setSettings(const BrushSettings &settings)
{
    const auto& s = settings;
    BasicBrush::setSettings(s);
    setWater(s.value("water", water_).toInt());
    setExtend(s.value("extend", extend_).toInt());
    setMixin(s.value("mixin", mixin_).toInt());
}

AbstractBrush *WaterBased::createBrush()
{
    return new WaterBased;
}
