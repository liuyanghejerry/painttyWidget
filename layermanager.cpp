#include "layermanager.h"

LayerManager::LayerManager()
    :lastSelected(0)
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

void LayerManager::removeLayer(const QString &name)
{
    if(!exists(name))return;
    if(layers[name]->isLocked()){
        qDebug()<<"Warning: try to remove locked layer";
        return;
    }
    layers.remove(name);
    layerLinks.remove(layerLinks.indexOf(name));
    qDebug()<<"remove"<<name;
}

void LayerManager::moveUp(const QString &name)
{
    //TODO
}

void LayerManager::moveDown(const QString &name)
{
    //TODO
}

void LayerManager::moveTo(int opos,int npos)
{
    //TODO
}

void LayerManager::moveTo(const QString &name, int npos)
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
     for(int i=0;i<layerLinks.count();++i){
         layers[layerLinks[i]]->resize(newsize);
     }
     qDebug()<<"LayerManager::resizeLayers:"<<newsize;
 }
