#include "layerlabel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QStyleOption>

LayerLabel::LayerLabel(QWidget *parent) :
    QWidget(parent),
    prefix_(tr("layer ")),
    isSelected_(false),
    isEdited_(false)
{
    setFocusPolicy(Qt::NoFocus);
}

QSize LayerLabel::sizeHint () const
{
    QSize t_size = size();
    t_size.setHeight(static_text_.size().toSize().height());
    return t_size;
}

QSize LayerLabel::minimumSizeHint () const
{
    QSize textSize = static_text_.size().toSize();
    return textSize+QSize(50,0);
}

void LayerLabel::setEditFlag(bool b)
{
    isEdited_ = b;
    update();
}

void LayerLabel::setText(const QString &string)
{
    text_ = string;
    static_text_.setText(prefix_+text_);
    static_text_.setTextFormat(Qt::PlainText);
    static_text_.prepare();
}

QString LayerLabel::text()
{
    return text_;
}

void LayerLabel::setSelected(bool s)
{
    isSelected_ = s;
    if(s)
        setEditFlag(s);

    update();
}

void LayerLabel::paintEvent (QPaintEvent *)
{

    QPainter painter(this);
    QStyleOption option;
    int midHeight = height()/2-static_text_.size().toSize().height()/2;
    painter.setPen(Qt::NoPen);
    QPen textPen;
    QBrush backgroundBrush;
    if(isSelected_){
        textPen = option.palette.highlightedText().color();
        backgroundBrush = option.palette.highlight();
    }else{
        textPen = option.palette.buttonText().color();
        backgroundBrush = option.palette.window();
    }
    painter.setBrush(backgroundBrush);
    painter.drawRect(rect());
    painter.setPen(textPen);
    painter.drawStaticText(5,midHeight,static_text_);

    painter.save();
    painter.setBrush(Qt::transparent);
    painter.drawEllipse(QPoint(width()-20, height()/2), 5, 5);
    if(isEdited_){
        painter.drawEllipse(QPoint(width()-20, height()/2), 3, 3);
    }
    painter.restore();
}
