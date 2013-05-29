#ifndef PLATFORMEXTEND_H
#define PLATFORMEXTEND_H

#include <QtGlobal>

class QWidget;

class PlatformExtend
{
public:
    PlatformExtend();
    static void notify(QWidget *w, int times = 2);
    static bool setIMEState(QWidget *widget, bool st);
    static bool supportTablet();
};

#ifdef Q_OS_WIN32
#include "tabletsupport_win32.h"
#else
#include "tabletsupport_dummy.h"
#endif

#endif // PLATFORMEXTEND_H
