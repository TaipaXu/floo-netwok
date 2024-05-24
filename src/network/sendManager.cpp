#include "./sendManager.hpp"

namespace Network
{
    SendManager *SendManager::getInstance()
    {
        static SendManager instance;
        return &instance;
    }

    SendManager::~SendManager()
    {
    }

    int SendManager::createSender(const QString &path)
    {
        Sender *sender = new Sender(this);
        senders.append(sender);
        emit sendersChanged();
        return sender->startSendFile(path);
    }
} // namespace Network
