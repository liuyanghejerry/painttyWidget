#include "platformextend.h"

void PlatformExtend::notify(QWidget *, int )
{
    //
}

bool PlatformExtend::setIMEState(QWidget *, bool st)
{
    return st;
}

bool PlatformExtend::supportTablet()
{
    return false;
}
