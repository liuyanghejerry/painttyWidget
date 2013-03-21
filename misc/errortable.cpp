#include "errortable.h"
#include <QObject>

//const std::unordered_map<int, QString> ErrorTable::errMsg_
//{
//    {200, QT_TR_NOOP("unknown error.")},
//    {201, QT_TR_NOOP("server is busy.")},
//    {202, QT_TR_NOOP("name collision.")},
//    {203, QT_TR_NOOP("invalid name.")},
//    {204, QT_TR_NOOP("invalid maxMember.")},
//    {205, QT_TR_NOOP("invalid welcomemsg.")},
//    {206, QT_TR_NOOP("invalid emptyclose.")},
//    {207, QT_TR_NOOP("invalid password.")},
//    {208, QT_TR_NOOP("emptyclose not supported.")},
//    {209, QT_TR_NOOP("private room not supported.")},
//    {210, QT_TR_NOOP("too many rooms.")},
//    {211, QT_TR_NOOP("invalid canvasSize.")},
//    {300, QT_TR_NOOP("unknown error.")},
//    {301, QT_TR_NOOP("invalid name.")},
//    {302, QT_TR_NOOP("invalid password or lack of password.")},
//    {303, QT_TR_NOOP("room is full.")},
//    {304, QT_TR_NOOP("you're banned.")},
//    {500, QT_TR_NOOP("closed by room or room manager.")},
//    {501, QT_TR_NOOP("closed by room owner.")},
//    {600, QT_TR_NOOP("unknown error.")},
//    {601, QT_TR_NOOP("room is closed already.")},

//};

//QString ErrorTable::toString(int errcode)
//{
//    if(errMsg_.at(errcode).isEmpty()){
//        return QT_TR_NOOP("unnamed error.");
//    }
//    return errMsg_.at(errcode);
//}

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
    case 500:
        return QObject::tr("closed by room or room manager.");
    case 501:
        return QObject::tr("closed by room owner.");
    case 600:
        return QObject::tr("unknown error.");
    case 601:
        return QObject::tr("room is closed already.");
    default:
        return QObject::tr("Unnamed error: %1.").arg(errorCode);
    }
}
