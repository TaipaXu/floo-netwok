#include "./channel.hpp"
#include <QFileInfo>

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

    void Channel::addFiles(const QList<QUrl> &myFiles)
    {
        for (const QUrl &url : myFiles)
        {
            QFileInfo fileInfo(url.toLocalFile());
            files.append(new MyFile(fileInfo.fileName(), fileInfo.size(), fileInfo.filePath(), this));
        }
        emit filesChanged();
    }
} // namespace Model
