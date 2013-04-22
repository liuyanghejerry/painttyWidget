#ifndef PAINTTY_CALL_ONCE_H
#define PAINTTY_CALL_ONCE_H

/***********
 * This header is fetched from
 * https://qt-project.org/wiki/Qt_thread-safe_singleton
 *
 *
 */


#include <QtCore/QtGlobal>
#include <QtCore/QAtomicInt>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QThreadStorage>
#include <QtCore/QThread>

namespace CallOnce {
    enum ECallOnce {
        CO_Request,
        CO_InProgress,
        CO_Finished
    };

    Q_GLOBAL_STATIC(QThreadStorage<QAtomicInt*>, once_flag)
}

template <class Function>
        inline static void qCallOnce(Function func, QBasicAtomicInt& flag)
{
    using namespace CallOnce;

    #if QT_VERSION < 0x050000
    int protectFlag = flag.fetchAndStoreAcquire(flag);
    #elif QT_VERSION >= 0x050000
    int protectFlag = flag.fetchAndStoreAcquire(flag.load());
    #endif

    if (protectFlag == CO_Finished)
        return;
    if (protectFlag == CO_Request && flag.testAndSetRelaxed(protectFlag,
                                                            CO_InProgress)) {
        func();
        flag.fetchAndStoreRelease(CO_Finished);
    }
    else {
        do {
            QThread::yieldCurrentThread();
        }
        while (!flag.testAndSetAcquire(CO_Finished, CO_Finished));
    }
}

template <class Function>
        inline static void qCallOncePerThread(Function func)
{
    using namespace CallOnce;
    if (!once_flag()->hasLocalData()) {
        once_flag()->setLocalData(new QAtomicInt(CO_Request));
        qCallOnce(func, *once_flag()->localData());
    }
}

#endif // PAINTTY_CALL_ONCE_H
