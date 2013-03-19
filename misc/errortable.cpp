#include "errortable.h"

const std::unordered_map<int, QString> ErrorTable::errMsg_
{
    {200, QT_TR_NOOP("unknown error.")},
    {201, QT_TR_NOOP("server is busy.")},
    {202, QT_TR_NOOP("name collision.")},
    {203, QT_TR_NOOP("invalid name.")},
    {204, QT_TR_NOOP("invalid maxMember.")},
    {205, QT_TR_NOOP("invalid welcomemsg.")},
    {206, QT_TR_NOOP("invalid emptyclose.")},
    {207, QT_TR_NOOP("invalid password.")},
    {208, QT_TR_NOOP("emptyclose not supported.")},
    {209, QT_TR_NOOP("private room not supported.")},
    {210, QT_TR_NOOP("too many rooms.")},
    {211, QT_TR_NOOP("invalid canvasSize.")},
    {300, QT_TR_NOOP("unknown error.")},
    {301, QT_TR_NOOP("invalid name.")},
    {302, QT_TR_NOOP("invalid password or lack of password.")},
    {303, QT_TR_NOOP("room is full.")},
    {304, QT_TR_NOOP("you're banned.")},
    {500, QT_TR_NOOP("closed by room or room manager.")},
    {501, QT_TR_NOOP("closed by room owner.")},
    {600, QT_TR_NOOP("unknown error.")},
    {601, QT_TR_NOOP("room is closed already.")},

};

QString ErrorTable::toString(int errcode)
{
    if(errMsg_.at(errcode).isEmpty()){
        return QT_TR_NOOP("unnamed error.");
    }
    return errMsg_.at(errcode);
}
