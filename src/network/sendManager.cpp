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
        // connect(sender, &Sender::statusChanged, [this, sender](Sender::Status status) {
        //     if (status == Sender::Status::Finished || status == Sender::Status::Error || status == Sender::Status::Canceled)
        //     {
        //         senders.removeOne(sender);
        //         sender->deleteLater();
        //     }
        // });
        senders.append(sender);
        return sender->startSendFile(path);
    }
} // namespace Network
