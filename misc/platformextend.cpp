#include "platformextend.h"
#include <QtGlobal>

#ifdef Q_OS_WIN32
#include "platformextend_win32.cpp"
#else
#include "platformextend_dummy.cpp"
#endif

