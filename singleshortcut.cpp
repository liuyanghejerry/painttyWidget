#include "singleshortcut.h"

SingleShortcut::SingleShortcut(QObject *parent) :
    QObject(parent),
    key_(0),
    enabled_(false)
{
    parent->installEventFilter(this);
}

bool SingleShortcut::setKey(int k)
{
    key_ = k;
    return true;
}

int SingleShortcut::key()
{
    return key_;
}

void SingleShortcut::setEnabled(bool e)
{
    enabled_ = e;
}

bool SingleShortcut::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( keyEvent->key() == key_
                && !keyEvent->isAutoRepeat()
                && keyEvent->modifiers() == Qt::NoModifier ){
            emit activated();
            return true;
        }
        return false;
    }else if(event->type() == QEvent::KeyRelease){
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( keyEvent->key() == key_
                && !keyEvent->isAutoRepeat()
                && keyEvent->modifiers() == Qt::NoModifier ){
            emit inactivated();
            return true;
        }
        return false;
    }else{
        return QObject::eventFilter(obj, event);
    }
}
