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
    features_ = bits;

    name_ = "Crayon";
    displayName_ = QObject::tr("Crayon");
    shortcut_ = Singleton<ShortcutManager>::instance()
            .shortcut("crayon")["key"].toString();
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
    QImage copied_stencil = stencil;
    bool need_delete = false;
    if(!painter) {
        painter = new QPainter;
        need_delete = true;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->begin(surface_->imagePtr());
    }

    int lineLength = copied_stencil.width();
    QRgb * data = (QRgb *)copied_stencil.bits();
    int mask_start_x = (p.x()+mask_.width()) % mask_.width();
    int mask_start_y = (p.y()+mask_.height()) % mask_.height();

    for(int y = 0; y<copied_stencil.height();++y){
        for(int x = 0; x<lineLength;++x) {
            int mask_x = (mask_start_x + x) % mask_.width();
            int mask_y = (mask_start_y + y) % mask_.height();
            if(!mask_.pixelIndex(mask_x, mask_y)) {
                data[y*lineLength+x] = qRgba(0, 0, 0, 0);
            }
        }
    }

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
    mask_ = mask.createAlphaMask();
    makeStencil(color_);
}

AbstractBrush *MaskBased::createBrush()
{
    return new MaskBased;
}


