#include "localnetworkinterface.h"
#include <QNetworkInterface>
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include "../common.h"

LocalNetworkInterface::LocalNetworkInterface()
{
}

bool LocalNetworkInterface::supportIpv6()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool useIpv6 = settings.value("global/ipv6", false).toBool();
    if (!useIpv6)
        return false;

    auto list = allAddress();
    for(auto &item : list){
        if(item.protocol() == QAbstractSocket::IPv6Protocol) {
            qDebug()<<"ipv6 support detected";
            return true;
        }
    }
    return false;
}

bool LocalNetworkInterface::supportIpv4()
{
    auto list = allAddress();
    for(auto &item : list){
        if(item.protocol() == QAbstractSocket::IPv4Protocol) {
            qDebug()<<"ipv4 support detected";
            return true;
        }
    }
    return false;
}

QList<QHostAddress> LocalNetworkInterface::allAddress()
{
    auto list = QNetworkInterface::allAddresses();
    for(auto &item : list){
        if(item.isLoopback()){
            list.removeAll(item);
        }
    }
    return list;
}
