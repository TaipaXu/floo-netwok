#pragma once

#include <QObject>
#include <QList>
#include "network/receiver.hpp"

namespace Network
{
    class ReceiveManager : public QObject
    {
        Q_OBJECT

    public:
        static ReceiveManager *getInstance();
        ~ReceiveManager();

        void createReceiver(const QString &ip, int port);

    private:
        ReceiveManager() = default;
        QList<Receiver *> receivers;
    };
} // namespace Network
