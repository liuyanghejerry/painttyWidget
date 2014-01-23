#include "layer.h"

#include <QImage>
#include <QColor>

Layer::Layer(const QString &name, const QSize &size)
    :lock_(false),
      hide_(false),
      select_(false),
      touched_(false),
      access_(true),
      name_(name),
      size_(size)
{
}

Layer::~Layer()
{
    img_.clear();
}

void Layer::create()
{
    img_ = QSharedPointer<QImage>(new QImage(size_, QImage::Format_ARGB32_Premultiplied));
    img_->fill(Qt::transparent);
    touched_ = true;
}

bool Layer::isLocked() const
{
    return lock_;
}

bool Layer::isHided() const
{
    return hide_;
}

bool Layer::isSelected() const
{
    return select_;
}

bool Layer::isTouched() const
{
    return touched_;
}

void Layer::lock()
{
    lock_ = true;
}

void Layer::unlock()
{
    lock_ = false;
}

void Layer::hide()
{
    hide_ = true;
}

void Layer::show()
{
    hide_ = false;
}

void Layer::select()
{
    select_ = true;
}

void Layer::deselect()
{
    select_ = false;
}

void Layer::clear()
{
    img_->fill(Qt::transparent);
    touched_ = false;
}

QImage* Layer::imagePtr()
{
    if(!touched_){
        create();
    }
    return img_.data();
}

const QImage* Layer::imageConstPtr()
{
    if(!touched_){
        create();
    }
    return img_.data();
}

void Layer::resize(const QSize &size)
{
    size_ = size;
    if(!img_.isNull()){
        if (img_->size() == size)
            return;
        *img_ = img_->scaled(size, Qt::KeepAspectRatio);
    }
}

QString Layer::name() const
{
    return name_;
}

void Layer::rename(const QString &new_name)
{
    name_ = new_name;
}
