#pragma once

#include <tuple>
#include <QObject>
#include <QList>
#include "network/sender.hpp"

namespace Network
{
    class SendManager : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QList<Sender *> senders READ getSenders NOTIFY sendersChanged)

    public:
        static SendManager *getInstance();
        ~SendManager() = default;

        int createSender(const QString &path);
        std::tuple<QString, int> createHttpSender(const QString &path);

    signals:
        void sendersChanged() const;

    private:
        SendManager() = default;

        const QList<Sender *> &getSenders() const;

    private:
        QList<Sender *> senders;
    };

    inline const QList<Sender *> &SendManager::getSenders() const
    {
        return senders;
    }
} // namespace Network

#define GET_SEND_MANAGER_INSTANCE SendManager *sendManager = Network::SendManager::getInstance();
