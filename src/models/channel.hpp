#pragma once

#include <QObject>
#include <QList>
#include <QUrl>
#include "models/file.hpp"
#include "models/myFile.hpp"

namespace Model
{
    class Channel : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Type type READ getType WRITE setType)
        Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(QString address READ getAddress WRITE setAddress NOTIFY addressChanged)
        Q_PROPERTY(int port READ getPort WRITE setPort NOTIFY portChanged)

    public:
        enum Type
        {
            Server,
            Client
        };
        Q_ENUM(Type)

    public:
        explicit Channel(QObject *parent = nullptr);
        Q_INVOKABLE Channel(Type type, const QString &name, const QString &address, int port, QObject *parent = nullptr);
        Q_INVOKABLE Channel(Type type, const QString &address, int port, QObject *parent = nullptr);
        ~Channel();

        Type getType() const;
        void setType(Type type);
        const QString &getName() const;
        void setName(const QString &name);
        const QString &getAddress() const;
        void setAddress(const QString &address);
        int getPort() const;
        void setPort(int port);

    signals:
        void nameChanged(const QString &name);
        void addressChanged(const QString &address);
        void portChanged(int port);

    private:
        Type channelType;
        QString channelName;
        QString channelAddress;
        int channelPort;
    };

    inline Channel::Type Channel::getType() const
    {
        return channelType;
    }

    inline const QString &Channel::getName() const
    {
        return channelName;
    }

    inline const QString &Channel::getAddress() const
    {
        return channelAddress;
    }

    inline int Channel::getPort() const
    {
        return channelPort;
    }
} // namespace Model
