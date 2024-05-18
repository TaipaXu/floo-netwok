#include "./receiveManager.hpp"

namespace Network
{
    ReceiveManager *ReceiveManager::getInstance()
    {
        static ReceiveManager instance;
        return &instance;
    }

    ReceiveManager::~ReceiveManager()
    {
    }

    void ReceiveManager::createReceiver(const QString &ip, int port)
    {
        Receiver *receiver = new Receiver(this);
        // connect(receiver, &Receiver::statusChanged, [this, receiver](Receiver::Status status) {
        //     if (status == Receiver::Status::Finished || status == Receiver::Status::Error || status == Receiver::Status::Canceled)
        //     {
        //         receivers.removeOne(receiver);
        //         receiver->deleteLater();
        //     }
        // });
        receiver->startReceiveFile(ip, port);
    }
} // namespace Network
