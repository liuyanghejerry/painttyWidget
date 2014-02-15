#include "networkindicator.h"
#include <QPainter>

NetworkIndicator::NetworkIndicator(QWidget *parent) :
    QWidget(parent),
    level_(UNKNOWN),
    display_text_(tr("- %1").arg("Unknown")),
    display_color_(112, 112, 112)
{
    setToolTip(tr("Network speed unknown"));
}

QSize NetworkIndicator::sizeHint() const
{
    return QSize(200, 20);
}

QSize NetworkIndicator::minimumSizeHint() const
{
    return QSize(200, 20);
}

void NetworkIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect r = this->rect();
    QPen pen;
    int radius = qMin(r.width(), r.height()) >> 1;
    pen.setWidth(radius);
    pen.setColor(display_color_);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.drawPoint(r.center());
    auto text_p = r.center() + QPoint(radius + 3, radius>>1);
    painter.drawText(text_p, display_text_);
}

NetworkIndicator::LEVEL NetworkIndicator::level() const
{
    return level_;
}

void NetworkIndicator::setLevel(const LEVEL &level)
{
    level_ = level;
    QString net_string;
    QString tooltip_string;
    switch(level_){
    case NONE:
        display_color_ = QColor(51, 51, 51);
        net_string = tr("Dead");
        tooltip_string = tr("Network sucks now");
        break;
    case LOW:
        display_color_ = QColor(194, 0, 48);
        net_string = tr("Bad");
        tooltip_string = tr("Network is very bad");
        break;
    case MEDIUM:
        display_color_ = QColor(245, 184, 0);
        net_string = tr("Medium");
        tooltip_string = tr("Network is ok");
        break;
    case GOOD:
        display_color_ = QColor(46, 184, 0);
        net_string = tr("Good");
        tooltip_string = tr("Excellent network");
        break;
    case UNKNOWN:
    default:
        display_color_ = QColor(112, 112, 112);
        net_string = tr("Unknown");
        tooltip_string = tr("Network speed unknown");
    }
    display_text_ = QString("- %1").arg(net_string);
    setToolTip(tooltip_string);
    update();
}

