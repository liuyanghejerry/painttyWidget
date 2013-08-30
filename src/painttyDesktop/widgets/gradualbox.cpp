#include "gradualbox.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>

GradualBox::GradualBox(QWidget *p)
    : QWidget(p),
      maxOpacity_(1.0),
      minOpacity_(0.0),
      opacity_(0),
      duration_(5000),
      autoPosition_(true),
      hideOnClick_(false),
      in_pressed_(false)
{
    setAttribute(Qt::WA_TranslucentBackground,true);
    setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip);
    setContentsMargins(30, 35, 30, 35);
}

GradualBox::GradualBox(const QString & pMessage, QWidget *p)
    : QWidget(p),
      maxOpacity_(1.0),
      minOpacity_(0.0),
      opacity_(0),
      duration_(5000),
      autoPosition_(true),
      hideOnClick_(false),
      in_pressed_(false)
{
    setAttribute(Qt::WA_TranslucentBackground,true);
    setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip);
    setText(pMessage);
}

GradualBox::~GradualBox()
{

}
qreal GradualBox::maxOpacity() const
{
    return maxOpacity_;
}

void GradualBox::setMaxOpacity(const qreal &maxOpacity)
{
    maxOpacity_ = qBound(0.0, maxOpacity, 1.0);
}
qreal GradualBox::minOpacity() const
{
    return minOpacity_;
}

void GradualBox::setMinOpacity(const qreal &minOpacity)
{
    minOpacity_ = qBound(0.0, minOpacity, 1.0);
}

void GradualBox::setOpacity(const qreal& op)
{
    opacity_ = qBound(0.0, op, 1.0);
    update();
}

int GradualBox::duration() const
{
    return duration_;
}

void GradualBox::setDuration(int duration)
{
    duration_ = duration;
}

void GradualBox::showText(const QString &msg,  bool hideOnClick, int duration)
{
    GradualBox* box = new GradualBox(0);
    connect(box, &GradualBox::finished,
            &GradualBox::deleteLater);
    box->setDuration(duration);
    box->setHideOnClick(hideOnClick);
    box->setText(msg);
}

void GradualBox::setText(const QString & pMessage)
{
    content_ = pMessage;
    prepare();

    this->show();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setDuration(duration_);

    if(hideOnClick_){
        this->setCursor(Qt::PointingHandCursor);
        animation->setDuration(duration_/4);
        animation->setKeyValueAt(0, minOpacity_);
        animation->setKeyValueAt(1.0, maxOpacity_);
        connect(this, &GradualBox::clicked,
                [animation, this](){
            animation->stop();
            animation->setKeyValueAt(1.0, opacity_);
            animation->setDirection(QAbstractAnimation::Backward);
            connect(animation, &QPropertyAnimation::finished,
                    this, &GradualBox::finished);
            disconnect(this, &GradualBox::clicked, 0, 0);
            animation->start();
        });
    }else{
        animation->setKeyValueAt(0, minOpacity_);
        animation->setKeyValueAt(0.3, maxOpacity_);
        animation->setKeyValueAt(0.7, maxOpacity_);
        animation->setKeyValueAt(1, minOpacity_);
        connect(animation, &QPropertyAnimation::finished,
                this, &GradualBox::finished);
    }
    animation->start();
}

void GradualBox::prepare()
{
    QFontMetrics fm(this->font());
    //Get the width of the text you want to show
    QMargins margins = contentsMargins();
    int mSide = (margins.left()+margins.right())/2;
    int mAround = (margins.top()+margins.bottom())/2;
    int mWidth = fm.width(content_)+mSide;
    int mHeight = fm.height()+mAround;
    resize( mWidth, mHeight);

    if(autoPosition_){
        QPoint src_center = QApplication::desktop()->screenGeometry().center();

        int mX = (src_center - QPoint((mWidth-mAround)/2,0)).x();
        int mY = (src_center - QPoint((mHeight-mSide)/2,0)).y();
        move(mX, mY*1.5);
    }
}
bool GradualBox::hideOnClick() const
{
    return hideOnClick_;
}

void GradualBox::setHideOnClick(bool hideOnClick)
{
    hideOnClick_ = hideOnClick;
}

bool GradualBox::autoPosition() const
{
    return autoPosition_;
}

void GradualBox::setAutoPosition(bool autoPosition)
{
    autoPosition_ = autoPosition;
}


void GradualBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen(Qt::transparent);
    QBrush brush(Qt::black);
    pen.setWidth(5);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setFont(this->font());
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setOpacity(opacity_);
    painter.drawRoundedRect(rect(), 10.0, 10.0);

    painter.setOpacity(qBound(0.0, opacity_+0.3, 1.0));
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawText(rect(),
                     content_,
                     QTextOption(Qt::AlignHCenter|Qt::AlignVCenter));
}

void GradualBox::mousePressEvent(QMouseEvent *)
{
    in_pressed_ = true;
}

void GradualBox::mouseReleaseEvent(QMouseEvent *)
{
    if(in_pressed_)
        emit clicked();
}
