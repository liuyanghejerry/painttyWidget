#ifndef TABLETSUPPORT_H
#define TABLETSUPPORT_H

#include <QObject>
#include <QAbstractNativeEventFilter>

struct tagAXIS {
};

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
protected:
    virtual bool nativeEventFilter(const QByteArray &eventType,
                                   void *message, long *) override;
};

#endif // TABLETSUPPORT_H
