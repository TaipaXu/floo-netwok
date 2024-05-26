#include "./sendManager.hpp"
#include "./httpSender.hpp"

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

    int SendManager::createHttpSender(const QString &path)
    {
        HttpSender *sender = new HttpSender(this);
        // senders.append(sender);
        // emit sendersChanged();
        return sender->startSendFile(path);
    }
} // namespace Network
