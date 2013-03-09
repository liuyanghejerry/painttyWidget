#ifndef SINGLESHORTCUT_H
#define SINGLESHORTCUT_H

#include <QObject>
#include <QKeyEvent>

class SingleShortcut : public QObject
{
    Q_OBJECT
public:
    explicit SingleShortcut(QObject *parent = 0);
    bool setKey(int k);
    int key();
    bool eventFilter(QObject *obj, QEvent *event);
    
signals:
    void activated();
    void inactivated();
    
public slots:
    void setEnabled(bool e);
private:
    int key_;
    bool enabled_;
    
};

#endif // SINGLESHORTCUT_H
