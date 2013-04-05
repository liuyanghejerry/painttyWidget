#ifndef LAYER_H
#define LAYER_H

#include <QSharedPointer>

class QPixmap;

class Layer
{
public:
    Layer(const QString &name, const QSize &size);
    ~Layer();
    QPixmap* imagePtr();
    const QPixmap* imageConstPtr();
    void resize(const QSize &size);
    bool isLocked();
    bool isHided();
    bool isSelected();
    void lock();
    void unlock();
    void hide();
    void show();
    void select();
    void deselect();
    void clear();
    QString name();
    void rename(const QString &new_name);
private:
    bool lock_;
    bool hide_;
    bool select_;
    bool access_;   //reserved
    QSharedPointer<QPixmap> img_;
    QString name_;
};

typedef QSharedPointer<Layer> LayerPointer;

#endif // LAYER_H
