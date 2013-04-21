#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>

namespace GlobalDef
{

const static char CLIENT_VER[] = "0.2";
const static char CLIENT_TYPE[] = "alpha";

const static char SETTINGS_NAME[] = "mrpaint.ini";

#ifdef PAINTTY_DEV
const static QString HOST_ADDR_IPV4("106.187.92.58");
const static QString HOST_ADDR_IPV6("106.187.92.58");
const static int HOST_MGR_PORT = 7070;
#else
const static QString HOST_ADDR_IPV4("106.187.92.58");
const static QString HOST_ADDR_IPV6("2400:8900::f03c:91ff:fe70:bc64");
const static int HOST_MGR_PORT = 7070;
#endif

const static QString HOST_ADDR[] = {HOST_ADDR_IPV4,
                                    HOST_ADDR_IPV6};

const qreal MAX_SCALE_FACTOR = 5.0;
const qreal MIN_SCALE_FACTOR = 0.125;

} // namespace GlobalDef

#endif // COMMON_H
