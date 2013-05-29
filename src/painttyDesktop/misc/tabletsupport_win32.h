#ifndef TABLETSUPPORT_H
#define TABLETSUPPORT_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include "wintabapi.h"

class TabletSupport : public QAbstractNativeEventFilter
{
public:
    TabletSupport(QWidget *window=nullptr);
    ~TabletSupport();
    bool hasDevice() const;
    QString deviceString() const;
    tagAXIS normalPressureInfo() const;
    tagAXIS tangentialPressureInfo() const;
    int eventRate() const;
    bool supportTilt() const;
    void start();
    void stop();
    const WinTabAPI& callFunc() const;
protected:
    virtual bool nativeEventFilter(const QByteArray &eventType,
                                   void *message, long *) override;
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
    HMODULE wintab_module;
    WinTabAPI tabapis;
    QObject msg_bumper;
    QWidget *window_;
    LOGCONTEXTA *logContext;
};

#endif // TABLETSUPPORT_H
