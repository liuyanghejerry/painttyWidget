#include "networkindicator.h"
#include <QPainter>

NetworkIndicator::NetworkIndicator(QWidget *parent) :
    QWidget(parent),
    level_(UNKNOWN)
{

}

QSize NetworkIndicator::sizeHint() const
{
    return QSize(150, 20);
}

QSize NetworkIndicator::minimumSizeHint() const
{
    return QSize(150, 20);
}

void NetworkIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect r = this->rect();
    QPen pen;
    QString net_string;
    int radius = qMin(r.width(), r.height()) >> 1;
    pen.setWidth(radius);
    switch(level_){
    case NONE:
        pen.setColor(QColor(51, 51, 51));
        net_string = tr("Dead");
        break;
    case LOW:
        pen.setColor(QColor(194, 0, 48));
        net_string = tr("Bad");
        break;
    case MEDIUM:
        pen.setColor(QColor(245, 184, 0));
        net_string = tr("Medium");
        break;
    case GOOD:
        pen.setColor(QColor(46, 184, 0));
        net_string = tr("Good");
        break;
    case UNKNOWN:
    default:
        pen.setColor(QColor(112, 112, 112));
        net_string = tr("Unknown");
    }
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.drawPoint(r.center());
    auto text_p = r.center() + QPoint(radius + 3, radius>>1);
    painter.drawText(text_p, QString("- %1").arg(net_string));
}

NetworkIndicator::LEVEL NetworkIndicator::level() const
{
    return level_;
}

void NetworkIndicator::setLevel(const LEVEL &level)
{
    level_ = level;
    update();
}

