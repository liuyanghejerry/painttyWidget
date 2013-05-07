#ifndef LOCALNETWORKINTERFACE_H
#define LOCALNETWORKINTERFACE_H

#include <QList>
#include <QHostAddress>

class LocalNetworkInterface
{
public:
    static bool supportIpv6();
    static bool supportIpv4();
    static QList<QHostAddress> allAddress();
private:
    LocalNetworkInterface();
};

#endif // LOCALNETWORKINTERFACE_H
