#pragma once

#include <QObject>
#include <QList>
#include "network/receiver.hpp"

namespace Network
{
    class ReceiveManager : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QList<Receiver *> receivers READ getReceivers NOTIFY receiversChanged)

    public:
        static ReceiveManager *getInstance();
        ~ReceiveManager();

        void createReceiver(const QString &ip, int port);

    signals:
        void receiversChanged() const;

    private:
        ReceiveManager() = default;

        const QList<Receiver *> &getReceivers() const;

    private:
        QList<Receiver *> receivers;
    };

    inline const QList<Receiver *> &ReceiveManager::getReceivers() const
    {
        return receivers;
    }
} // namespace Network
