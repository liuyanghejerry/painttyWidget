#include "layermanager.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>

LayerManager::LayerManager(const QSize &initSize)
    :lastSelected(0),
      layerSize_(initSize)
{
}

LayerPointer LayerManager::layerFrom(int pos)
{
    if( pos >= layers.count() ){
        return LayerPointer();
    }
    return layers[layerLinks[pos]];
}

LayerPointer LayerManager::layerFrom(const QString &name)
{
    if(!exists(name)){
        qDebug()<<"Warnning: try to access a non-existent layer";
        return LayerPointer();
    }
    return layers[name];
}

LayerPointer LayerManager::topLayer()
{
    return layers[layerLinks.last()];
}

LayerPointer LayerManager::bottomLayer()
{
    return layers[layerLinks.first()];
}

void LayerManager::updateSelected()
{
    for(int i=0;i<layerLinks.count();++i){
        LayerPointer l = layers[layerLinks[i]];
        if(l->isSelected() && l!=lastSelected){
            lastSelected->deselect();
            lastSelected = l;
        }
    }
}

LayerPointer LayerManager::selectedLayer()
{
    return lastSelected;
}

LayerPointer LayerManager::topShownLayer()
{
    for(int i=layerLinks.count()-1;i>0;--i){
        if(layers[layerLinks[i]]->isHided()){
            continue;
        }else{
            return layers[layerLinks[i]];
        }
    }
    qWarning()<<"topShownLayer() returns null ptr";
    return LayerPointer();
}

void LayerManager::select(const QString &name)
{
    if(exists(name)){
        LayerPointer l = layers[name];
        l->select();
        if(lastSelected) lastSelected->deselect();
        lastSelected = l;
    }else{
        qDebug()<<"Selected an non-exists layer";
    }
}

void LayerManager::insertLayer(LayerPointer image, const QString &name, int pos)
{
    layerLinks.insert(pos,name);
    layers.insert(name,image);
    qDebug()<<"instert"<<name<<"at"<<pos;
}

void LayerManager::appendLayer(LayerPointer image, const QString &name)
{
    if(layers.contains(name)){
        qDebug()<<"Dupli";
    }
    layerLinks.append(name);
    layers.insert(name,image);
    qDebug()<<"append"<<name<<"at"<<(layerLinks.count()-1);
}

LayerPointer LayerManager::appendLayer(const QString &name)
{
    if(layers.contains(name)){
        qWarning()<<"Duplicated layer skipped!";
        return LayerPointer();
    }
    layerLinks.append(name);
    LayerPointer lp(new Layer(name, layerSize_));
    layers.insert(name, lp);
    qDebug()<<"append"<<name<<"at"<<(layerLinks.count()-1);
    return lp;
}

void LayerManager::removeLayer(const QString &name)
{
    if(!exists(name))return;
    if(layers[name]->isLocked()){
        qDebug()<<"Warning: try to remove locked layer";
        return;
    }
    layers.remove(name);
    layerLinks.removeAll(name);
    qDebug()<<"remove"<<name;
}

void LayerManager::clearLayer(const QString &name)
{
    if(!exists(name)) return;
    layers[name]->clear();
    qDebug()<<"clear content of"<<name;
}

void LayerManager::clearAllLayer()
{
    for(auto &item: layers.values()){
        item->clear();
    }
    qDebug()<<"all layers cleared";
}

void LayerManager::moveUp(const QString &)
{
    //TODO
}

void LayerManager::moveDown(const QString &)
{
    //TODO
}

void LayerManager::moveTo(int ,int )
{
    //TODO
}

void LayerManager::moveTo(const QString &, int )
{
    //TODO
}

bool LayerManager::exists(const QString &name)
{
    return layers.contains(name);
}

bool LayerManager::exists(int pos)
{
    return layerLinks.count()-1 >pos;
}

void LayerManager::rename(const QString &oname,const QString &nname)
{
    if(layers.contains(oname)){
        layers[nname] = layers[oname];
        layers[oname] = LayerPointer();
        int i = layerLinks.indexOf(oname);
        layerLinks[i] = nname;
    }
}

void LayerManager::resizeLayers(const QSize &newsize)
{
    layerSize_ = newsize;
    for(int i=0;i<layerLinks.count();++i){
        layers[layerLinks[i]]->resize(layerSize_);
    }
    qDebug()<<"LayerManager::resizeLayers:"<<layerSize_;
}

void LayerManager::combineLayers(QPixmap *p, const QRect &rect)
{
    *p = p->scaled(layerSize_);
    p->fill(Qt::white);
    QPainter painter(p);
    int lc = this->count();
    QPixmap * im = 0;
    for(int i=0;i<lc;++i){
        LayerPointer l = layerFrom(i);
        if( l->isHided() || !l->isTouched() ){
            continue;
        }
        im = l->imagePtr();
        if(rect.isNull()){
            painter.drawPixmap(0, 0, *im);
        }else{
            painter.drawPixmap(QRectF(rect), *im, QRectF(rect));
        }
    }
}
