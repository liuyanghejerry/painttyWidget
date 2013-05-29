#include "tabletsupport.h"
#include <QWidget>
#include <QTabletEvent>
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QDebug>

TabletSupport::TabletSupport(QWidget *window):
{
    
}

TabletSupport::~TabletSupport()
{
}

void TabletSupport::start()
{

}

void TabletSupport::stop()
{

}

bool TabletSupport::hasDevice() const
{
    return false;
}

QString TabletSupport::deviceString() const
{
    return QString();
}

tagAXIS TabletSupport::normalPressureInfo() const
{
	return tagAXIS a;
}

tagAXIS TabletSupport::tangentialPressureInfo() const
{
    return tagAXIS a;
}

int TabletSupport::eventRate() const
{
    return 0;
}

bool TabletSupport::supportTilt() const
{
    return false
}

bool TabletSupport::nativeEventFilter(const QByteArray &eventType,
                                      void *message, long *)
{
    return false;
}
