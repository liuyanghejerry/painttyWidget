#include "layer.h"

#include <QPixmap>

Layer::Layer(const QSize &size)
    :lock_(false),
      hide_(false),
      select_(false),
      access_(true)
{
    img_ = QSharedPointer<QPixmap>(new QPixmap(size));
    img_->fill(Qt::transparent);
}

Layer::~Layer()
{
    img_.clear();
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
}

QPixmap* Layer::imagePtr()
{
    return img_.data();
}

const QPixmap* Layer::imageConstPtr()
{
    return img_.data();
}

void Layer::resize(const QSize &size)
{
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
