#ifndef SINGLESHORTCUT_H
#define SINGLESHORTCUT_H

#include <QObject>
#include <QKeyEvent>
#include <QKeySequence>

class SingleShortcut : public QObject
{
    Q_OBJECT
public:
    explicit SingleShortcut(QObject *parent = 0);
    void setKey(int k);
    void setKey(QKeySequence ks);
    QKeySequence key();
    bool eventFilter(QObject *obj, QEvent *event);
    
signals:
    void activated();
    void inactivated();
    
public slots:
    void setEnabled(bool e);
private:
    QKeySequence key_;
    bool enabled_;
    
};

#endif // SINGLESHORTCUT_H
