#include "./utils.hpp"
#include <QNetworkInterface>
#include <QUuid>
#include <sstream>
#include <iomanip>

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

QStringList Utils::getLocalAddresses()
{
    QStringList addresses;
    for (auto &&interface : QNetworkInterface::allInterfaces())
    {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            for (auto &&address : interface.addressEntries())
            {
                if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    addresses.append(address.ip().toString());
                }
            }
        }
    }
    return addresses;
}
