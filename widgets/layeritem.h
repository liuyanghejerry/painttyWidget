#ifndef LAYERITEM_H
#define LAYERITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QFocusEvent>
#include "iconcheckbox.h"
#include "layerlabel.h"

class LayerItem : public QWidget
{
    Q_OBJECT
public:
    explicit LayerItem(QWidget *parent = 0);
    QString label();
    void setVisibleIcon(const QIcon &icon);
    void setLockIcon(const QIcon &icon);
    void setLabel(const QString &string);
    void setSelect(bool b);
    bool isHide();
    bool isLock();
    
signals:
    void selected();
    void deSelected();
    void lock(bool);
    void hide(bool);
    
public slots:
protected:
    void paintEvent(QPaintEvent *);
    void focusInEvent( QFocusEvent * event );
private:
    IconCheckBox *visible_;
    IconCheckBox *lock_;
    LayerLabel *label_;    
};

#endif // LAYERITEM_H
