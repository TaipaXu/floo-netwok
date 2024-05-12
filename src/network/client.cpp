#include "./client.hpp"
#include <QTcpSocket>

namespace Network
{
    Client::Client(QObject *parent)
        : QObject(parent), tcpSocket{nullptr}
    {
    }

    Client::~Client()
    {
        qDebug() << "client destructor";
        if (tcpSocket)
        {
            stop();
        }
    }

    void Client::start(const QString &address, int port)
    {
        qDebug() << "start" << address << port;
        if (!tcpSocket)
        {
            tcpSocket = new QTcpSocket(this);
            connect(tcpSocket, &QTcpSocket::connected, this, &Client::handleConnected);
            connect(tcpSocket, &QTcpSocket::readyRead, this, &Client::handleReadyRead);
            connect(tcpSocket, &QTcpSocket::errorOccurred, this, &Client::handleError);
            connect(tcpSocket, &QTcpSocket::disconnected, this, &Client::handleDisconnected);
        }
        tcpSocket->connectToHost(address, port);
    }

    void Client::stop()
    {
        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }
    }

    void Client::handleConnected() const
    {
        qDebug() << "client connected";
        emit connected();
    }

    void Client::handleReadyRead()
    {
        const QByteArray message = tcpSocket->readAll();
        qDebug() << "client received message" << message;
    }

    void Client::handleError(QAbstractSocket::SocketError socketError) const
    {
        qDebug() << "client error" << socketError;
        emit connectError();
    }

    void Client::handleDisconnected()
    {
        qDebug() << "client disconnected";
        emit disconnected();

        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }
    }
} // namespace Network
