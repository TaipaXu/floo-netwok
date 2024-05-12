#include "./server.hpp"
#include <QTcpServer>
#include <QTcpSocket>
#include "models/file.hpp"
#include "models/myFile.hpp"

namespace Network
{
    Server::Server(QObject *parent)
        : QObject(parent), tcpServer{nullptr}
    {
    }

    Server::~Server()
    {
        qDebug() << "server destructor";
        if (tcpServer)
        {
            stop();
        }
    }

    bool Server::start(const QString &address, int port)
    {
        qDebug() << "start" << address << port;
        if (!tcpServer)
        {
            tcpServer = new QTcpServer(this);
            connect(tcpServer, &QTcpServer::newConnection, this, &Server::handleNewConnection);
            connect(tcpServer, &QTcpServer::acceptError, [this](QAbstractSocket::SocketError socketError) {
                qDebug() << "server accept error" << socketError;
            });
        }

        bool result = tcpServer->listen(QHostAddress(address), port);
        if (result)
        {
            qDebug() << "Server started at" << address << port;
        }
        else
        {
            qDebug() << "Server start failed";
        }
        return result;
    }

    void Server::stop()
    {
        if (tcpServer)
        {
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }

        socketFiles.clear();
    }

    void Server::handleNewConnection()
    {
        qDebug() << "server new connection";
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::handleDisconnected);
        socketFiles[socket] = std::list<std::shared_ptr<Model::File>>();
        emit newConnection();
    }

    void Server::handleReadyRead()
    {
        qDebug() << "server received message";
        QTcpSocket *const socket = qobject_cast<QTcpSocket *>(sender());
        if (socket)
        {
            QByteArray data = socket->readAll();
            qDebug() << "data" << data;
        }
    }

    void Server::handleDisconnected()
    {
        qDebug() << "server disconnected";
        emit disconnected();

        if (tcpServer)
        {
            QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
            if (socket)
            {
                socket->deleteLater();
                socketFiles.erase(socket);
            }
        }
    }
} // namespace Network
