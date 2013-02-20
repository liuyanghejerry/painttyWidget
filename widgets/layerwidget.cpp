#include "layerwidget.h"

LayerWidget::LayerWidget(QWidget *parent) :
    QWidget(parent),
    scrollArea_(0),
    header_(0),
    layout_(0),
    lastSelected_(0)
{
    this->setLayout(new QVBoxLayout);

    scrollArea_ = new QScrollArea(this);

    //disable header
    //    header_ = new LayerWidgetHeader;
    //    this->layout()->addWidget(header_);

    this->layout()->addWidget(scrollArea_);

    QWidget *wid = new QWidget(scrollArea_);
    layout_ = new QVBoxLayout;
    wid->setLayout(layout_);
    layout_->addStretch(1);
    layout_->setContentsMargins(0,0,0,0);
    layout_->setMargin(0);
    layout_->setSpacing(0);

    scrollArea_->setWidget(wid);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


}

QSize LayerWidget::sizeHint () const
{
    return QSize(width(), height());
}

QSize LayerWidget::minimumSizeHint () const
{
    return QSize(100, 20);
}

int LayerWidget::itemCount()
{
    return layout_->count();
}

LayerItem* LayerWidget::itemAt(int index)
{
    if(itemCount()){
        return qobject_cast<LayerItem*>(layout_->itemAt(index)->widget());
    }else{
        return 0;
    }
}

void LayerWidget::addItem(LayerItem *item)
{
    layout_->insertWidget(0,item);
    connect(item,SIGNAL(selected()),this,SLOT(itemSelected()));
    connect(item,SIGNAL(hide(bool)),this,SLOT(itemHidden()));
    connect(item,SIGNAL(lock(bool)),this,SLOT(itemLocked()));
}

void LayerWidget::removeItem(LayerItem *item)
{
    layout_->removeWidget(item);
    //    disconnect(item,SIGNAL(selected()),this,SLOT(itemSelected()));
}

void LayerWidget::removeItem(const QString &name)
{
    for(int i=0;i<layout_->count();++i){
        LayerItem *item = qobject_cast<LayerItem *>(layout_->itemAt(i)->widget());
        if(item && item->label() == name){
            layout_->removeWidget(item);
            disconnect(item,SIGNAL(selected()),this,SLOT(itemSelected()));
        }
    }
}

void LayerWidget::itemSelected()
{
    LayerItem *item = qobject_cast<LayerItem *>(sender());
    if(!item) return;

    if(lastSelected_ && lastSelected_ != item){
        lastSelected_->setSelect(false);
    }
    lastSelected_ = item;
    emit itemSelected(item->label());
}

void LayerWidget::itemHidden()
{
    LayerItem *item = qobject_cast<LayerItem *>(sender());
    if(!item) return;

    bool h = item->isHide();
    QString s = item->label();
    if(h){
        emit itemHide(s);
    }else{
        emit itemShow(s);
    }
}

void LayerWidget::itemLocked()
{
    LayerItem *item = qobject_cast<LayerItem *>(sender());
    if(!item) return;

    bool l = item->isLock();
    QString s = item->label();
    if(l){
        emit itemLock(s);
    }else{
        emit itemUnlock(s);
    }
}

LayerItem* LayerWidget::selected()
{
    return lastSelected_;
}
