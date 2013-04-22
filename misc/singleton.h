#ifndef SINGLETON_H
#define SINGLETON_H

/***********
 * This header is fetched from
 * https://qt-project.org/wiki/Qt_thread-safe_singleton
 *
 * Usage: Singleton<MyClass>::instance();
 *
 */

#include <QtCore/QtGlobal>
#include <QtCore/QScopedPointer>
#include "call_once.h"

template <class T>
class Singleton
{
public:
    static T& instance()
    {
        qCallOnce(init, flag);
        return *tptr;
    }

    static void init()
    {
        tptr.reset(new T);
    }

private:
    Singleton() {}
    ~Singleton() {}
    Q_DISABLE_COPY(Singleton)

    static QScopedPointer<T> tptr;
    static QBasicAtomicInt flag;
};

template<class T> QScopedPointer<T> Singleton<T>::tptr(0);
template<class T> QBasicAtomicInt Singleton<T>::flag
                            = Q_BASIC_ATOMIC_INITIALIZER(CallOnce::CO_Request);

#endif // SINGLETON_H
