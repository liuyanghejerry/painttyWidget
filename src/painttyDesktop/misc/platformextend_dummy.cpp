#include "platformextend.h"
#include <QApplication>

void PlatformExtend::notify(QWidget *widget, int times)
{
     QApplication::alert(widget, times);
}

bool PlatformExtend::setIMEState(QWidget *, bool st)
{
    return st;
}
