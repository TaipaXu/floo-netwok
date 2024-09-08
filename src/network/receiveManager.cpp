#include "./receiveManager.hpp"
#include "network/httpReceiver.hpp"

namespace Network
{
    ReceiveManager *ReceiveManager::getInstance()
    {
        static ReceiveManager instance;
        return &instance;
    }

    void ReceiveManager::createTcpReceiver(const QString &ip, int port)
    {
        TcpReceiver *receiver = new TcpReceiver(this);
        receiver->startReceiveFile(ip, port);
        receivers.append(receiver);
        emit receiversChanged();
    }

    std::tuple<QString, int> ReceiveManager::createHttpReceiver()
    {
        HttpReceiver *receiver = new HttpReceiver(this);
        receivers.append(receiver);
        emit receiversChanged();
        return receiver->startReceiveFile();
    }
} // namespace Network
