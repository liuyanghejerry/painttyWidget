#include "waterbased.h"
#include <QtGlobal>
#include <functional>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QBitmap>
#include <QObject>
#include <QDebug>
#include <cmath>

#include "../../misc/shortcutmanager.h"
#include "../../misc/singleton.h"

typedef BrushFeature::LIMIT BFL;

static inline void print_color(QColor c, QString name)
{
    qDebug()<<name
           <<c.red()
          <<c.green()
         <<c.blue()
        <<c.alpha();
}

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

    name_ = "WaterBrush";
    displayName_ = QObject::tr("WaterBrush");
    shortcut_ = Singleton<ShortcutManager>::instance()
            .shortcut("waterbrush")["key"].toString();
}
int WaterBased::water() const
{
    return water_;
}

void WaterBased::setWater(int water)
{
    water_ = qBound<int>(BFL::WATER_MIN, water, BFL::WATER_MAX);
}
int WaterBased::extend() const
{
    return extend_;
}

void WaterBased::setExtend(int extend)
{
    extend_ = qBound<int>(BFL::EXTEND_MIN, extend, BFL::EXTEND_MAX);
}
int WaterBased::mixin() const
{
    return mixin_;
}

void WaterBased::setMixin(int mixin)
{
    mixin_ = qBound<int>(BFL::MIXIN_MIN, mixin, BFL::MIXIN_MAX);
}

// non-reentrant.
static inline QImage circle_mask(const int width)
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

// avg_rgb treats transparent points as white
static inline QRgb avg_rgb(const QImage& square)
{
    QRegion region(0, 0, square.width(), square.height(), QRegion::Ellipse);
    bool isPremultiplied = square.format() == QImage::Format_ARGB32_Premultiplied;
    QRgb target = qRgba(255, 255, 255, 0);
    const QRgb* img_data = (const QRgb*) square.constBits();
    const size_t img_data_len = square.byteCount() >> (sizeof(QRgb)>>1);

    unsigned int r_sum = 0;
    unsigned int g_sum = 0;
    unsigned int b_sum = 0;
    unsigned int a_sum = 0;
    unsigned int a_calc_sum = 0;

    unsigned int r = 255;
    unsigned int g = 255;
    unsigned int b = 255;
    unsigned int a = 0;
    unsigned int a_calc = 0;

    size_t colored = 0;
    for(size_t i = 0;i<img_data_len;++i){
        if(!region.contains(QPoint(i/square.width(), i%square.width()))){
            continue;
        }
        QRgb next = *(img_data+i);
        a_calc = a = qAlpha(next);
        if(!a){
            r = 255;
            g = 255;
            b = 255;
            a_calc = 255;
        }else{
            if(isPremultiplied){
                r = qRed(next);
                g = qGreen(next);
                b = qBlue(next);
            }else{
                r = qRed(next)*a/255;
                g = qGreen(next)*a/255;
                b = qBlue(next)*a/255;
            }
        }

        ++colored;

        r_sum += r;
        g_sum += g;
        b_sum += b;
        a_sum += a;
        a_calc_sum += a_calc;
    }
    target = qRgba(r_sum *255/a_calc_sum ,
                   g_sum *255/a_calc_sum,
                   b_sum *255/a_calc_sum,
                   a_sum / colored);
    return target;
}

// avg_rgb2 does not caculate transparent points
static inline QRgb avg_rgb2(const QImage& square)
{
    QRegion region(0, 0, square.width(), square.height(), QRegion::Ellipse);
    bool isPremultiplied = square.format() == QImage::Format_ARGB32_Premultiplied;
    QRgb target = qRgba(0, 0, 0, 0);
    const QRgb* img_data = (const QRgb*) square.constBits();
    const size_t img_data_len = square.byteCount() >> (sizeof(QRgb)>>1);

    unsigned int r_sum = 0;
    unsigned int g_sum = 0;
    unsigned int b_sum = 0;
    unsigned int a_sum = 0;

    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;
    unsigned int a = 0;

    size_t colored = 0;
    for(size_t i = 0;i<img_data_len;++i){
        if(!region.contains(QPoint(i/square.width(), i%square.width()))){
            continue;
        }
        QRgb next = *(img_data+i);
        a = qAlpha(next);
        if(!a){
            continue;
        }else{
            if(isPremultiplied){
                r = qRed(next)*255/a;
                g = qGreen(next)*255/a;
                b = qBlue(next)*255/a;
            }else{
                r = qRed(next);
                g = qGreen(next);
                b = qBlue(next);
            }
        }

        ++colored;

        r_sum += r;
        g_sum += g;
        b_sum += b;
        a_sum += a;
    }
    if(!colored) {
        return qRgba(0, 0, 0, 0);
    }
    target = qRgba(r_sum / colored ,
                   g_sum / colored,
                   b_sum / colored,
                   a_sum / colored);
    return target;
}

static inline QColor watering_color(int water, QColor color)
{
//    color.setAlpha((BFL::WATER_MAX - water) * color.alpha() / BFL::WATER_MAX);
//    return color;
    auto proc = [&water](int v) -> int {
        return qBound(0,
                      v + water*255 / 100,
                      255);
    };

    int r = proc(color.red());
    int g = proc(color.green());
    int b = proc(color.blue());
    return QColor(r, g, b, color.alpha());
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

    const QRgb c = avg_rgb2(square);
    return QColor::fromRgba(c);
}

inline static QColor mingle_color(QColor c1, QColor c2, int percent, int max)
{
    int r = qBound(0,
                   (c1.red() * percent + c2.red() * (max - percent)) / max,
                   255);
    int g = qBound(0,
                   (c1.green() * percent + c2.green() * (max - percent)) / max,
                   255);
    int b = qBound(0,
                   (c1.blue() * percent + c2.blue() * (max - percent)) / max,
                   255);
    int a = qBound(0,
                   (c1.alpha() * percent + c2.alpha() * (max - percent)) / max,
                   255);

    return QColor(r, g, b, a);
    //    return QColor(r, g, b);
}

static inline QColor mixin_color(QColor brush_color, QColor paper_color, int mixin)
{
    return mingle_color(paper_color, brush_color, mixin, BFL::MIXIN_MAX);
}

static inline QColor extend_color(QColor origin, QColor paper, int remain)
{
    return mingle_color(origin, paper, remain, 255);
}

void WaterBased::drawPoint(const QPoint &p, qreal )
{
    last_color_ = fetchColor(p);
    last_point_ = p;
    color_remain_ = 255;
}

void WaterBased::drawLineTo(const QPoint &end, qreal presure)
{
    if(end.x() > surface_->imageConstPtr()->width() || end.x() < 0
            || end.y() > surface_->imageConstPtr()->height() || end.y() < 0) {
        return;
    }
    const QPoint& start = last_point_;
//    last_color_ = fetchColor(last_point_);
    // TODO: spacing needs to be calc with thickness and hardness, too
    const qreal spacing = width_*presure*0.04;

    const qreal deltaX = end.x() - start.x();
    const qreal deltaY = end.y() - start.y();

    const qreal distance = sqrt( deltaX * deltaX + deltaY * deltaY );
    qreal stepX = 0.0;
    qreal stepY = 0.0;
    if ( distance > 0.0 ) {
        qreal invertDistance = 1.0 / distance;
        stepX = deltaX * invertDistance;
        stepY = deltaY * invertDistance;
    }

    qreal offsetX = 0.0;
    qreal offsetY = 0.0;

    qreal seg_X = 0.0;
    qreal seg_Y = 0.0;

    qreal totalDistance = left_ + distance;

    QPainter painter(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    while ( totalDistance >= spacing ) {
        bool l_f_ = false;
        if ( left_ > 0.0 ) {
            l_f_ = true;
            seg_X = stepX * (spacing - left_);
            seg_Y = stepY * (spacing - left_);
            left_ -= spacing;
        } else {
            seg_X = stepX * spacing;
            seg_Y = stepY * spacing;
        }

        offsetX += seg_X;
        offsetY += seg_Y;

        QPoint cur_point(start.x() + offsetX - (stencil_.width()>>1),
                         start.y() + offsetY - (stencil_.height()>>1));

        if(l_f_){
            makeStencil(last_color_);
        }else{
            last_color_ = fetchColor(cur_point);

            int length = distance;
            color_remain_ -= (length >>1) * (BFL::EXTEND_MAX-extend_)/BFL::EXTEND_MAX;
            color_remain_ = qBound(0, color_remain_, 255);

            auto watered_color = watering_color(water_, color_);
            auto extended_color = extend_color(watered_color, last_color_, color_remain_);
            auto mixed_color = mixin_color(extended_color, last_color_, mixin_);
            makeStencil(mixed_color);
        }

        drawPointInternal(cur_point,
                          stencil_,
                          &painter);

        totalDistance -= spacing;
    }
    left_ = totalDistance;
    last_point_ = end;
}

void WaterBased::setSettings(const BrushSettings &settings)
{
    const auto& s = settings;
    setWater(s.value("water", water_).toInt());
    setExtend(s.value("extend", extend_).toInt());
    setMixin(s.value("mixin", mixin_).toInt());
    BasicBrush::setSettings(s);
}

BrushSettings WaterBased::defaultSettings() const
{
    auto s = BasicBrush::defaultSettings();
    s.insert("water", 50);
    s.insert("extend", 50);
    s.insert("mixin", 20);
    return s;
}

AbstractBrush *WaterBased::createBrush()
{
    return new WaterBased;
}
