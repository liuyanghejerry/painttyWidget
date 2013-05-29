#include "platformextend.h"

#include <QWidget>
#include <QDebug>
#include "windows.h"
#include "Imm.h"

void PlatformExtend::notify(QWidget *widget, int times)
{
    HWND hwnd = (HWND)(widget->winId());
    DWORD dwFlags = FLASHW_TRAY;
    UINT  uCount = times;
    DWORD dwTimeout = 0;
    UINT  cbSize = sizeof(FLASHWINFO);
    FLASHWINFO info;
    info.cbSize = cbSize;
    info.hwnd = hwnd;
    info.dwFlags = dwFlags;
    info.uCount = uCount;
    info.dwTimeout = dwTimeout;
    qDebug()<<"flash!";
    FlashWindowEx(&info);
}

bool PlatformExtend::setIMEState(QWidget *widget, bool st)
{
    HWND handle = (HWND)(widget->winId());
    auto hImc =  ImmGetContext(handle);

    qDebug()<<"IME state set to"<<(st?"open":"close");
    // returns non-zero when success
    return ImmSetOpenStatus(hImc, st);
}

bool PlatformExtend::supportTablet()
{
    return true;
}
