#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>

namespace GlobalDef
{

const static char CLIENT_VER[] = "0.1";
const static char CLIENT_TYPE[] = "alpha";

const static char SETTINGS_NAME[] = "mrpaint.ini";

#ifdef PAINTTY_DEV
const static char HOST_ADDR[] = "42.121.85.47";
const static int HOST_MGR_PORT = 7070;
#else
const static char HOST_ADDR[] = "42.121.85.47";
const static int HOST_MGR_PORT = 3030;
#endif

const qreal MAX_SCALE_FACTOR = 5.0;
const qreal MIN_SCALE_FACTOR = 0.125;

} // namespace GlobalDef

#endif // COMMON_H
