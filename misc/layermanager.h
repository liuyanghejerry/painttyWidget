#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QList>
#include <QHash>
#include "layer.h"

class QString;
typedef QSharedPointer<Layer> LayerPointer;

class LayerManager
{
public:

    LayerManager();
    LayerPointer layerFrom(int pos);
    LayerPointer layerFrom(const QString &name);
    LayerPointer topLayer();
    LayerPointer bottomLayer();
    LayerPointer selectedLayer();
    LayerPointer topShownLayer();
    void select(const QString &name);
    void insertLayer(LayerPointer image,
                     const QString &name, int pos);
    void appendLayer(LayerPointer image, const QString &name);
    void removeLayer(const QString &name);
    void clearLayer(const QString &name);
    void clearAllLayer();
    void moveUp(const QString &name);
    void moveDown(const QString &name);
    void moveTo(const QString &, int);
    bool exists(const QString &name);
    bool exists(int pos);
    void rename(const QString &oname, const QString &nname);
    int count(){return layers.count();}
    void resizeLayers(const QSize &newsize);
    void updateSelected();

private:
    void moveTo(int, int);

    QList<QString> layerLinks;
    QHash<QString, LayerPointer> layers;
    LayerPointer lastSelected;

};

#endif // LAYERMANAGER_H
