#include "maskbased.h"
#include <QPainter>
#include <QDebug>

#include "../../misc/shortcutmanager.h"
#include "../../misc/singleton.h"

MaskBased::MaskBased() :
    BasicBrush()
{
    typedef BrushFeature BF;
    BF::FeatureBits bits;
    bits.set(BF::WIDTH);
    bits.set(BF::COLOR);
    bits.set(BF::THICKNESS);
    bits.set(BF::HARDNESS);
    features_ = bits;

    name_ = "Crayon";
    displayName_ = QObject::tr("Crayon");
    shortcut_ = Singleton<ShortcutManager>::instance()
            .shortcut("crayon")["key"].toString();
    this->setMask(QImage(":/iconset/canvas-print.png"));
}

void MaskBased::makeStencil(QColor color)
{
    BasicBrush::makeStencil(color);
    if(mask_.isNull()){
        return;
    }
}

void MaskBased::drawPointInternal(const QPoint &p, const QImage &stencil, QPainter *painter)
{
    QImage copied_stencil = stencil.convertToFormat(QImage::Format_ARGB32);
    bool need_delete = false;
    if(!painter) {
        painter = new QPainter;
        need_delete = true;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->begin(surface_->imagePtr());
//        painter->setOpacity(thickness_/qreal(BrushFeature::LIMIT::THICKNESS_MAX));
    }

    int lineLength = copied_stencil.width();
    QRgb * data = (QRgb *)copied_stencil.bits();
    int mask_start_x = (p.x()+mask_.width()) % mask_.width();
    int mask_start_y = (p.y()+mask_.height()) % mask_.height();

//    for(int y = 0; y<copied_stencil.height();++y){
//        for(int x = 0; x<lineLength;++x) {
//            int mask_x = (mask_start_x + x) % mask_.width();
//            int mask_y = (mask_start_y + y) % mask_.height();
//            data[y*lineLength+x] = qRgba(
//                        qRed(data[y*lineLength+x]),
//                        qGreen(data[y*lineLength+x]),
//                        qBlue(data[y*lineLength+x]),
//                        qAlpha(mask_.pixel(mask_x, mask_y)));
//        }
//    }

    for(int y = 0;y<copied_stencil.height();++y){
        for(int x = 0; x<copied_stencil.width();++x){
            int mask_x = (mask_start_x + x) % mask_.width();
            int mask_y = (mask_start_y + y) % mask_.height();
            auto oc = copied_stencil.pixel(x, y);
            auto c = qRgba(
                        qRed(oc),
                        qGreen(oc),
                        qBlue(oc),
                        qAlpha(mask_.pixel(mask_x, mask_y)));
            copied_stencil.setPixel(x, y, c);
        }
    }

//    copied_stencil.save(QString("./d/%1-%2.png").arg(p.x()).arg(p.y()));

    painter->drawImage(p.x(), p.y(), copied_stencil);

    if(need_delete) {
        painter->end();
        delete painter;
    }
}
QImage MaskBased::mask() const
{
    return mask_;
}

void MaskBased::setMask(const QImage &mask)
{
    if(mask.isNull()){
        qDebug()<<"null mask";
        return;
    }
//    mask_ = mask.createAlphaMask();
    mask_ = mask.convertToFormat(QImage::Format_ARGB32);
    makeStencil(color_);
}

AbstractBrush *MaskBased::createBrush()
{
    return new MaskBased;
}


