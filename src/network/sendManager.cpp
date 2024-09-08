#include "./sendManager.hpp"
#include "./httpSender.hpp"

namespace Network
{
    SendManager *SendManager::getInstance()
    {
        static SendManager instance;
        return &instance;
    }

    int SendManager::createTcpSender(const QString &path)
    {
        TcpSender *const sender = new TcpSender(this);
        senders.append(sender);
        emit sendersChanged();
        return sender->startSendFile(path);
    }

    std::tuple<QString, int> SendManager::createHttpSender(const QString &path)
    {
        HttpSender *const sender = new HttpSender(this);
        senders.append(sender);
        emit sendersChanged();
        return sender->startSendFile(path);
    }
} // namespace Network
