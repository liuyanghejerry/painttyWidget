#include "iconcheckbox.h"

#include <QPainter>
#include <QStyleOption>
#include <QBrush>

IconCheckBox::IconCheckBox(QWidget *parent) :
    QAbstractButton(parent),
    size_(iconSize())
{
    setCheckable(true);
    setAutoRepeat(false);
    resize(size_);
    connect(this,&IconCheckBox::toggled,
            this,static_cast<void (IconCheckBox::*) ()>(&IconCheckBox::update));
//    connect(this,SIGNAL(pressed()),this,SLOT(update()));
}

QSize IconCheckBox::sizeHint () const
{
    return QSize(size_);
}

void IconCheckBox::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QStyleOption option;
    if(icon().isNull()){
        return;
    }

    QSize icon_size = iconSize();

    if(isChecked()){
        painter.drawPixmap(0,0,icon().pixmap(icon_size,QIcon::Normal,QIcon::On));
    }else if(isDown()){
        painter.drawPixmap(0,0,icon().pixmap(icon_size,QIcon::Active,QIcon::On));
    }

    //draw 3d box
    int box_width = icon_size.width();
    int box_height = icon_size.height();
    QPen pen(option.palette.color(QPalette::Shadow));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(0,0,box_width-1,0);
    painter.drawLine(0,0,0,box_height-1);
    pen.setColor(option.palette.color(QPalette::Light));
    painter.setPen(pen);
    painter.drawLine(1,box_height-1,box_width-1,box_height-1);
    painter.drawLine(box_width-1,1,box_width-1,box_height-1);
}

void IconCheckBox::enterEvent(QEvent *)
{
    setCursor(Qt::PointingHandCursor);
}

void IconCheckBox::leaveEvent(QEvent *)
{
    setCursor(Qt::ArrowCursor);
}

bool IconCheckBox::hitButton ( const QPoint &  ) const
{
    return true;
}
