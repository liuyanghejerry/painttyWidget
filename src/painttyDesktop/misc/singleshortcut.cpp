#include "singleshortcut.h"
#include <QPainter>
#include <QKeyEvent>

SingleShortcut::SingleShortcut(QObject *parent) :
    QObject(parent),
    enabled_(false)
{
    parent->installEventFilter(this);
}

void SingleShortcut::setKey(int k)
{
    key_ = QKeySequence(k);
}

void SingleShortcut::setKey(QKeySequence ks)
{
    key_ = ks;
}

QKeySequence SingleShortcut::key()
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
        if( QKeySequence(keyEvent->key()).matches(key_)
                == QKeySequence::ExactMatch
                && !keyEvent->isAutoRepeat()
                && keyEvent->modifiers() == Qt::NoModifier ){
            emit activated();
            return true;
        }
        return false;
    }else if(event->type() == QEvent::KeyRelease){
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( QKeySequence(keyEvent->key()).matches(key_)
                == QKeySequence::ExactMatch
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
