#include "roomsharebar.h"
#include <QLabel>
#include <QLineEdit>
#include <QSizePolicy>
#include <QHBoxLayout>

#include "easycopylineedit.h"

RoomShareBar::RoomShareBar(QWidget *parent) :
    QWidget(parent),
    roomShareLB_(new QLabel(tr("Share your room:"), this)),
    roomAddrLE_(new EasyCopyLineEdit(this)),
    layout_(new QHBoxLayout(this))
{
    this->setLayout(layout_);
    layout_->setMargin(0);

    roomAddrLE_->setReadOnly(true);
    roomAddrLE_->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                           QSizePolicy::Minimum,
                                           QSizePolicy::LineEdit));
    layout_->addWidget(roomShareLB_);
    layout_->addWidget(roomAddrLE_);
}

void RoomShareBar::setAddress(const QString &t)
{
    roomAddrLE_->setText(t);
}

QString RoomShareBar::address() const
{
    return roomAddrLE_->text();
}
