#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QMetaMethod>
#include <QString>
#include <QTimer>
#include <type_traits>
#include <QDebug>

namespace GlobalDef
{

const static char CLIENT_VER[] = "0.41";
const static char CLIENT_TYPE[] = "alpha";
static const char UPDATER_VERSION[] = "0.12";

const static char SETTINGS_NAME[] = "mrpaint.ini";

#ifdef PAINTTY_DEV
const static QString HOST_ADDR_IPV4("192.81.128.133");
const static QString HOST_ADDR_IPV6("2600:3c01::f03c:91ff:fe70:bc64");
const static QString UPDATER_ADDR_IPV4("42.121.85.47");
const static QString UPDATER_ADDR_IPV6("42.121.85.47");
const static int HOST_MGR_PORT = 7575;
const static int UPDATER_PORT = 7071;
#else
const static QString HOST_ADDR_IPV4("192.81.128.133");
const static QString HOST_ADDR_IPV6("2600:3c01::f03c:91ff:fe70:bc64");
const static QString UPDATER_ADDR_IPV4("42.121.85.47");
const static QString UPDATER_ADDR_IPV6("42.121.85.47");
const static int HOST_MGR_PORT = 7070;
const static int UPDATER_PORT = 7071;
#endif

const static QString HOST_ADDR[] = {HOST_ADDR_IPV4,
                                    HOST_ADDR_IPV6};



const static QString UPDATER_ADDR[] = {UPDATER_ADDR_IPV4,
                                    UPDATER_ADDR_IPV6};

#if defined(Q_OS_WIN32)
static const char* DOWNLOAD_URL = "http://download.mrspaint.com/0.4/%E8%8C%B6%E7%BB%98%E5%90%9B_Alpha_x86.zip";
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACX)
static const char* DOWNLOAD_URL = "";
#elif defined(Q_OS_MACX)
static const char* DOWNLOAD_URL = "http://download.mrspaint.com/0.4/%E8%8C%B6%E7%BB%98%E5%90%9B_Alpha_Mac_0.4.zip";
#endif

const qreal MAX_SCALE_FACTOR = 5.0;
const qreal MIN_SCALE_FACTOR = 0.125;

template<typename Func>
void delayJob(Func f, int ms=2000)
{
    if( std::is_function<decltype(f)>::value
            || std::is_object<decltype(f)>::value){
        QTimer* t = new QTimer;
        t->setSingleShot(true);
        QObject::connect(t, &QTimer::timeout,
                [&f, t](){
            qDebug()<<"delayJob";
            f();
            t->deleteLater();
        });
        t->start(ms);
    }
}

} // namespace GlobalDef

#endif // COMMON_H
