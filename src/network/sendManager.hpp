#pragma once

#include <QObject>
#include <QList>
#include "network/sender.hpp"

namespace Network
{
    class SendManager : public QObject
    {
        Q_OBJECT

    public:
        static SendManager *getInstance();
        ~SendManager();

        int createSender(const QString &path);

    private:
        SendManager() = default;

        QList<Sender *> senders;
    };
} // namespace Network
