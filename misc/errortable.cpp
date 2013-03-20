#include "errortable.h"

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
        return QT_TR_NOOP("unknown error.");
    case 201:
        return QT_TR_NOOP("server is busy.");
    case 202:
        return QT_TR_NOOP("name collision.");
    case 203:
        return QT_TR_NOOP("invalid name.");
    case 204:
        return QT_TR_NOOP("invalid maxMember.");
    case 205:
        return QT_TR_NOOP("invalid welcomemsg.");
    case 206:
        return QT_TR_NOOP("invalid emptyclose.");
    case 207:
        return QT_TR_NOOP("invalid password.");
    case 208:
        return QT_TR_NOOP("emptyclose not supported.");
    case 209:
        return QT_TR_NOOP("private room not supported.");
    case 210:
        return QT_TR_NOOP("too many rooms.");
    case 211:
        return QT_TR_NOOP("invalid canvasSize.");
    case 300:
        return QT_TR_NOOP("unknown error.");
    case 301:
        return QT_TR_NOOP("invalid name.");
    case 302:
        return QT_TR_NOOP("invalid password or lack of password.");
    case 303:
        return QT_TR_NOOP("room is full.");
    case 304:
        return QT_TR_NOOP("you're banned.");
    case 500:
        return QT_TR_NOOP("closed by room or room manager.");
    case 501:
        return QT_TR_NOOP("closed by room owner.");
    case 600:
        return QT_TR_NOOP("unknown error.");
    case 601:
        return QT_TR_NOOP("room is closed already.");
    default:
        return QString(QT_TR_NOOP("Unnamed error: %1.")).arg(errorCode);
    }
}
