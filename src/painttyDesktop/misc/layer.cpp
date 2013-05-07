#include "layer.h"

#include <QPixmap>

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
    img_ = QSharedPointer<QPixmap>(new QPixmap(size_));
    img_->fill(Qt::transparent);
    touched_ = true;
}

bool Layer::isLocked()
{
    return lock_;
}

bool Layer::isHided()
{
    return hide_;
}

bool Layer::isSelected()
{
    return select_;
}

bool Layer::isTouched()
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
    img_.clear();
    touched_ = false;
}

QPixmap* Layer::imagePtr()
{
    if(!touched_){
        create();
    }
    return img_.data();
}

const QPixmap* Layer::imageConstPtr()
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

QString Layer::name()
{
    return name_;
}

void Layer::rename(const QString &new_name)
{
    name_ = new_name;
}
