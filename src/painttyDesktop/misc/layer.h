#ifndef LAYER_H
#define LAYER_H

#include <QSharedPointer>
#include <QSize>

class QImage;

class Layer
{
public:
    Layer(const QString &name, const QSize &size);
    ~Layer();
    QImage* imagePtr();
    const QImage* imageConstPtr();
    void resize(const QSize &size);
    bool isLocked() const;
    bool isHided() const;
    bool isSelected() const;
    bool isTouched() const;
    void lock();
    void unlock();
    void hide();
    void show();
    void select();
    void deselect();
    void clear();
    QString name() const;
    void rename(const QString &new_name);
private:
    Q_DISABLE_COPY(Layer)
    bool lock_;
    bool hide_;
    bool select_;
    bool touched_;  // if pixmap is already created
    bool access_;   //reserved
    QSharedPointer<QImage> img_;
    QString name_;
    QSize size_;
    void create();
};

typedef QSharedPointer<Layer> LayerPointer;

#endif // LAYER_H
