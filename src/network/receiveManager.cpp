#include "./receiveManager.hpp"
#include "network/httpReceiver.hpp"

namespace Network
{
    ReceiveManager *ReceiveManager::getInstance()
    {
        static ReceiveManager instance;
        return &instance;
    }

    void ReceiveManager::createReceiver(const QString &ip, int port)
    {
        Receiver *receiver = new Receiver(this);
        receiver->startReceiveFile(ip, port);
        receivers.append(receiver);
        emit receiversChanged();
    }

    std::tuple<QString, int> ReceiveManager::createHttpReceiver()
    {
        HttpReceiver *receiver = new HttpReceiver(this);
        return receiver->startReceiveFile();
    }
} // namespace Network
