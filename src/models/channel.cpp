#include "./channel.hpp"
#include <QtDebug>

namespace Model
{
    Channel::Channel(QObject *parent)
        : QObject(parent)
    {
    }

    Channel::Channel(Channel::Type type, const QString &name, const QString &address, int port, bool webEnabled, QObject *parent)
        : QObject(parent),
          channelType(type),
          channelName(name),
          channelAddress(address),
          channelPort(port),
          webEnabled(webEnabled)
    {
    }

    Channel::Channel(Channel::Type type, const QString &address, int port, bool webEnabled, QObject *parent)
        : QObject(parent),
          channelType(type),
          channelAddress(address),
          channelPort(port),
          webEnabled(webEnabled)
    {
    }

    Channel::~Channel()
    {
        qDebug() << "Channel destructor";
    }

    void Channel::setType(Type type)
    {
        channelType = type;
    }

    void Channel::setName(const QString &name)
    {
        channelName = name;
        emit nameChanged(name);
    }

    void Channel::setAddress(const QString &address)
    {
        channelAddress = address;
        emit addressChanged(address);
    }

    void Channel::setPort(int port)
    {
        channelPort = port;
        emit portChanged(channelPort);
    }

    void Channel::setWebEnabled(bool enabled)
    {
        qDebug() << "setWebEnabled" << enabled;
        webEnabled = enabled;
        emit webEnabledChanged(enabled);
    }

    void Channel::setWsPort(int port)
    {
        wsPort = port;
    }

    void Channel::setWebServerPort(int port)
    {
        webServerPort = port;
    }
} // namespace Model
