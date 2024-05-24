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
        receiver->startReceiveFile(ip, port);
        receivers.append(receiver);
        emit receiversChanged();
    }
} // namespace Network
