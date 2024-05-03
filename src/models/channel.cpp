#include "./channel.hpp"

namespace Model
{
    Channel::Channel(QObject *parent)
        : QObject(parent)
    {
    }

    Channel::Channel(Channel::Type type, const QString &name, const QString &address, int port, QObject *parent)
        : QObject(parent),
          channelType(type),
          channelName(name),
          channelAddress(address),
          channelPort(port)
    {
    }

    Channel::Channel(Channel::Type type, const QString &address, int port, QObject *parent)
        : QObject(parent),
          channelType(type),
          channelAddress(address),
          channelPort(port)
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
} // namespace Model
