#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>

namespace GlobalDef
{

const static char CLIENT_VER[] = "0.3";
const static char CLIENT_TYPE[] = "alpha";
static const char UPDATER_VERSION[] = "0.1";

const static char SETTINGS_NAME[] = "mrpaint.ini";

#ifdef PAINTTY_DEV
const static QString HOST_ADDR_IPV4("106.187.53.78");
const static QString HOST_ADDR_IPV6("2400:8900::f03c:91ff:fe70:bc64");
const static int HOST_MGR_PORT = 7070;
const static int HOST_UPDATER_PORT = 7071;
#else
const static QString HOST_ADDR_IPV4("106.187.53.78");
const static QString HOST_ADDR_IPV6("2400:8900::f03c:91ff:fe70:bc64");
const static int HOST_MGR_PORT = 7070;
const static int HOST_UPDATER_PORT = 7071;
#endif

const static QString HOST_ADDR[] = {HOST_ADDR_IPV4,
                                    HOST_ADDR_IPV6};
									
#ifdef Q_OS_WIN32
static const char* DOWNLOAD_URL = "http://mrspaint.oss.aliyuncs.com/%E8%8C%B6%E7%BB%98%E5%90%9B_Alpha_x86.zip";
#elif Q_OS_UNIX
static const char* DOWNLOAD_URL = "";
#elif Q_OS_MAC
static const char* DOWNLOAD_URL = "http://mrspaint.oss.aliyuncs.com/%E8%8C%B6%E7%BB%98%E5%90%9B.app.zip";
#endif

const qreal MAX_SCALE_FACTOR = 5.0;
const qreal MIN_SCALE_FACTOR = 0.125;

} // namespace GlobalDef

#endif // COMMON_H
