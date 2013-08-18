#ifndef LAYER_H
#define LAYER_H

#include <QSharedPointer>
#include <QSize>

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
    bool isTouched();
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
    Q_DISABLE_COPY(Layer)
    bool lock_;
    bool hide_;
    bool select_;
    bool touched_;  // if pixmap is already created
    bool access_;   //reserved
    QSharedPointer<QPixmap> img_;
    QString name_;
    QSize size_;
    void create();
};

typedef QSharedPointer<Layer> LayerPointer;

#endif // LAYER_H
