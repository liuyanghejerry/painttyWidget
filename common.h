#ifndef COMMON_H
#define COMMON_H

namespace GlobalDef {

const static char CLIENT_VER[] = "0.1";
const static char CLIENT_TYPE[] = "alpha";

const static char SETTINGS_NAME[] = "mrpaint.ini";

#ifdef DEBUG
const static char HOST_ADDR[] = "192.168.1.111";
const static int HOST_MGR_PORT = 7070;
#else
const static char HOST_ADDR[] = "42.121.85.47";
const static int HOST_MGR_PORT = 10086;
#endif
}

#endif // COMMON_H
