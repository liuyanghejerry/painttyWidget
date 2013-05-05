#include "layeritem.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QFocusEvent>
#include <QPainter>
#include <QStyleOption>
#include "iconcheckbox.h"
#include "layerlabel.h"

LayerItem::LayerItem(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    visible_ = new IconCheckBox(this);
    lay->addWidget(visible_);
    visible_->setChecked(true);

    lock_ = new IconCheckBox(this);
    lay->addWidget(lock_);

    label_ = new LayerLabel(this);
    lay->addWidget(label_, 1);
    this->setLayout(lay);

    setFocusPolicy(Qt::ClickFocus);

    int m = lay->margin();
    lay->setContentsMargins(m,m-5,0,m-5);

    connect(lock_,&IconCheckBox::toggled,
            this, &LayerItem::lock);
    connect(visible_,&IconCheckBox::toggled,
            this, &LayerItem::hide);
}

void LayerItem::setVisibleIcon(const QIcon &icon)
{
    visible_->setIcon(icon);
}

void LayerItem::setLockIcon(const QIcon &icon)
{
    lock_->setIcon(icon);
}

void LayerItem::setLabel(const QString &string)
{
    label_->setText(string);
}

QString LayerItem::label()
{
    return label_->text();
}

void LayerItem::setSelect(bool b)
{
    label_->setSelected(b);
    label_->setEditFlag(b);
    update();
    if(b){
        emit selected();
    }else{
        emit deSelected();
    }
}

bool LayerItem::isHide()
{
    return !visible_->isChecked();
}

bool LayerItem::isLock()
{
    return lock_->isChecked();
}

void LayerItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QStyleOption option;
    QPen borderPen;
    borderPen.setColor(option.palette.text().color());
    QBrush bgBrush = option.palette.window();
    painter.setPen(borderPen);
    painter.setBrush(bgBrush);

    painter.drawLine(0,height()-1,width(),height()-1);
}

void LayerItem::focusInEvent( QFocusEvent * event )
{
    if(event->gotFocus()){
        setSelect(true);
    }
}
