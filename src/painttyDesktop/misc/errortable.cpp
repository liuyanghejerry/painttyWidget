#include "errortable.h"
#include <QObject>

QString ErrorTable::toString(int errorCode)
{
    switch (errorCode)
    {
    case 200:
        return QObject::tr("unknown error.");
    case 201:
        return QObject::tr("server is busy.");
    case 202:
        return QObject::tr("name collision.");
    case 203:
        return QObject::tr("invalid name.");
    case 204:
        return QObject::tr("invalid max member.");
    case 205:
        return QObject::tr("invalid welcome words.");
    case 206:
        return QObject::tr("invalid auto closing empty room.");
    case 207:
        return QObject::tr("invalid password.");
    case 208:
        return QObject::tr("auto closing empty room not supported.");
    case 209:
        return QObject::tr("private room not supported.");
    case 210:
        return QObject::tr("too many rooms.");
    case 211:
        return QObject::tr("invalid canvas size.");
    case 300:
        return QObject::tr("unknown error.");
    case 301:
        return QObject::tr("invalid name.");
    case 302:
        return QObject::tr("invalid password or lack of password.");
    case 303:
        return QObject::tr("room is full.");
    case 304:
        return QObject::tr("you're banned.");
    case 305:
        return QObject::tr("server is too busy.");
    case 500:
        return QObject::tr("closed by room or room manager.");
    case 501:
        return QObject::tr("closed by room owner.");
    case 600:
        return QObject::tr("unknown error.");
    case 601:
        return QObject::tr("room is closed already.");
    case 700:
        return QObject::tr("unknown error.");
    case 701:
        return QObject::tr("wrong key.");
    case 702:
        return QObject::tr("timeout, which means too late.");
    case 800:
        return QObject::tr("unknown error.");
    case 900:
        return QObject::tr("unknown error.");
    case 901:
        return QObject::tr("out of range.");
    default:
        return QObject::tr("Unnamed error: %1.").arg(errorCode);
    }
}
