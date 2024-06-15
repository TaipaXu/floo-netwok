#include "./channel.hpp"

namespace Model
{
    Channel::Channel(QObject *parent)
        : QObject(parent)
    {
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
