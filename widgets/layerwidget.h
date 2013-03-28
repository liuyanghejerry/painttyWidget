#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include <QWidget>

class LayerItem;
class QScrollArea;
class LayerWidgetHeader;
class QVBoxLayout;


class LayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LayerWidget(QWidget *parent = 0);
    int itemCount();
    LayerItem* itemAt(int index);
    void addItem(LayerItem *item);
    void removeItem(LayerItem *item);
    void removeItem(const QString &name);
    LayerItem* selected();
    QSize sizeHint () const;
    QSize minimumSizeHint () const;
signals:
    void itemSelected(QString);
    void itemHide(QString);
    void itemShow(QString);
    void itemLock(QString);
    void itemUnlock(QString);
    
private slots:
    void onItemSelected();
    void onItemHidden();
    void onItemLocked();
private:
    QScrollArea *scrollArea_;
    LayerWidgetHeader *header_;
    QVBoxLayout *layout_;
    LayerItem *lastSelected_;
};

#endif // LAYERWIDGET_H
