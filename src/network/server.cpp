#include "./server.hpp"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
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

        qDeleteAll(connections);
        connections.clear();
    }

    void Server::handleNewConnection()
    {
        qDebug() << "server new connection";
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::handleDisconnected);
        connections.push_back(new Model::Connection(socket, this));
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
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(data, &jsonError);
            if (!json.isNull() && jsonError.error == QJsonParseError::NoError)
            {
                const QJsonObject obj = json.object();
                const QString type = obj.value("type").toString();
                if (type == "files")
                {
                    qDebug() << "files";
                    const QJsonArray filesArray = obj.value("files").toArray();
                    addClientFiles(socket, filesArray);
                }
            }
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
                connections.erase(std::remove_if(connections.begin(), connections.end(), [socket](Model::Connection *connection) {
                                      return connection->getTcpSocket() == socket;
                                  }),
                                  connections.end());
            }
        }
    }

    void Server::addClientFiles(QTcpSocket *socket, const QJsonArray &filesArray)
    {
        qDebug() << "add client files";
        Model::Connection *connection = nullptr;
        for (Model::Connection *conn : connections)
        {
            if (conn->getTcpSocket() == socket)
            {
                connection = conn;
                break;
            }
        }

        if (connection)
        {
            QList<Model::File *> files;
            for (const QJsonValue &fileValue : filesArray)
            {
                const QJsonObject fileObj = fileValue.toObject();
                const QString id = fileObj.value("code").toString();
                const QString name = fileObj.value("name").toString();
                const int size = fileObj.value("path").toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
        }
    }
} // namespace Network
