#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QList>
#include <QHash>
#include <QSize>
#include <QRect>
#include "layer.h"

class QString;
typedef QSharedPointer<Layer> LayerPointer;

class LayerManager
{
public:

    LayerManager(const QSize &initSize);
    LayerPointer layerFrom(int pos) const;
    LayerPointer layerFrom(const QString &name) const;
    LayerPointer topLayer() const;
    LayerPointer bottomLayer() const;
    LayerPointer selectedLayer() const;
    LayerPointer topShownLayer() const;
    void select(const QString &name);
    void insertLayer(LayerPointer image,
                     const QString &name, int pos);
    void appendLayer(LayerPointer image, const QString &name);
    LayerPointer appendLayer(const QString &name);
    void removeLayer(const QString &name);
    void clearLayer(const QString &name);
    void clearAllLayer();
    void moveUp(const QString &name);
    void moveDown(const QString &name);
    void moveTo(const QString &, int);
    bool exists(const QString &name) const;
    bool exists(int pos) const;
    void rename(const QString &oname, const QString &nname);
    int count() const{return layers.count();}
    void resizeLayers(const QSize &newsize);
    void updateSelected();
    void combineLayers(QImage *p, const QRect &rect = QRect());

private:
    Q_DISABLE_COPY(LayerManager)
    void moveTo(int, int);

    QList<QString> layerLinks;
    QHash<QString, LayerPointer> layers;
    LayerPointer lastSelected;
    QSize layerSize_;

};

#endif // LAYERMANAGER_H
