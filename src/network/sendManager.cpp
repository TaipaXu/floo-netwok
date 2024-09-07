#include "./sendManager.hpp"
#include "./httpSender.hpp"

namespace Network
{
    SendManager *SendManager::getInstance()
    {
        static SendManager instance;
        return &instance;
    }

    int SendManager::createSender(const QString &path)
    {
        Sender *const sender = new Sender(this);
        senders.append(sender);
        emit sendersChanged();
        return sender->startSendFile(path);
    }

    std::tuple<QString, int> SendManager::createHttpSender(const QString &path)
    {
        HttpSender *const sender = new HttpSender(this);
        return sender->startSendFile(path);
    }
} // namespace Network
