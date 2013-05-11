#ifndef TABLETSUPPORT_H
#define TABLETSUPPORT_H

#include <QObject>
#include "wintabapi.h"
#include "eventcaptor.h"
class EventCaptor;

class TabletSupport : public QObject
{
    Q_OBJECT
public:
    TabletSupport(QWidget *window=nullptr);
    ~TabletSupport();
    bool hasDevice() const;
    QString deviceString() const;
    tagAXIS normalPressureInfo() const;
    tagAXIS tangentialPressureInfo() const;
    int eventRate() const;
    bool supportTilt() const;
    const WinTabAPI& callFunc() const;
    void start();
    void stop();
private:
    bool loadWintab();
    bool freeWintab();
    template<typename T1, typename T2=const char*>
    bool getProcAddr(T1& funcAddr, T2 funcName)
    {
        funcAddr = (T1) GetProcAddress(wintab_module, funcName);
        if (!funcAddr){
            freeWintab();
            return false;
        }
        return true;
    }
    bool mapWintabFuns();
    void captureEvent();
    HMODULE wintab_module;
    WinTabAPI tabapis;
    QObject msg_bumper;
    QWidget *window_;
    LOGCONTEXTA *logContext;
    EventCaptor captor_;
    friend class EventCaptor;
};

#endif // TABLETSUPPORT_H
