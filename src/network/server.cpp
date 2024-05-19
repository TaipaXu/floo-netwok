#include "./server.hpp"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include "network/sendManager.hpp"
#include "network/receiveManager.hpp"

namespace Network
{
    Server::Server(QObject *parent)
        : QObject(parent), tcpServer{nullptr}
    {
    }

    Server::~Server()
    {
        qDebug() << "server destructor";
        stop();
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

        for (auto &&connection : connections)
        {
            connection->deleteLater();
        }
        connections.clear();

        emit connectionsChanged();
        broadcastFiles();
    }

    void Server::broadcastFiles() const
    {
        qDebug() << "broadcast files";
        if (myFiles.empty() && connections.empty())
        {
            return;
        }

        QJsonObject json;
        json["type"] = "files";
        QJsonObject ipFiles;
        ipFiles["server"] = Model::toJson(myFiles);
        for (Model::Connection *connection : connections)
        {
            ipFiles[connection->getAddress()] = Model::toJson(connection->getFiles());
        }
        json["files"] = ipFiles;

        const QJsonDocument doc(json);
        const QByteArray data = doc.toJson(QJsonDocument::Compact);
        for (Model::Connection *connection : connections)
        {
            connection->getTcpSocket()->write(data);
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
                const QString id = fileObj.value("id").toString();
                const QString name = fileObj.value("name").toString();
                const int size = fileObj.value("path").toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
        }

        broadcastFiles();
    }

    void Server::handleClientRequestDownloadFile(QTcpSocket *clientSocket, const QString &id)
    {
        qDebug() << "handle client request download file";
        for (auto &&file : myFiles)
        {
            if (file->getId() == id)
            {
                const int port = Network::SendManager::getInstance()->createSender(file->getPath());
                QJsonObject json;
                json["type"] = "uploadFileReady";
                json["id"] = id;
                json["ip"] = tcpServer->serverAddress().toString();
                json["port"] = port;
                clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                break;
            }
        }

        for (Model::Connection *connection : connections)
        {
            for (auto &&file : connection->getFiles())
            {
                if (file->getId() == id)
                {
                    QJsonObject json;
                    json["type"] = "prepareUpload";
                    json["id"] = id;
                    json["ip"] = connection->getAddress();
                    clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    break;
                }
            }
        }
    }

    void Server::handleClientReadyToUploadFile(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId)
    {
        qDebug() << "handle client ready to upload file";
        if (reveiverIp == "server")
        {
            qDebug() << "server ready to upload file" << senderIp << reveiverIp << port;
            Network::ReceiveManager::getInstance()->createReceiver(senderIp, port);
        }
        else
        {
            for (Model::Connection *connection : connections)
            {
                if (connection->getAddress() == reveiverIp)
                {
                    QJsonObject json;
                    json["type"] = "uploadFileReady";
                    json["id"] = fileId;
                    json["ip"] = senderIp;
                    json["port"] = port;
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    break;
                }
            }
        }
    }

    void Server::addMyFiles(const QList<QUrl> &myFiles)
    {
        qDebug() << "Server::addMyFiles";
        for (const QUrl &url : myFiles)
        {
            QFileInfo fileInfo(url.toLocalFile());
            this->myFiles.append(new Model::MyFile(fileInfo.fileName(), fileInfo.size(), fileInfo.filePath(), this));
        }

        emit myFilesChanged();
        broadcastFiles();
    }

    void Server::removeMyFile(Model::MyFile *myFile)
    {
        qDebug() << "Server::removeMyFile";
        myFiles.removeOne(myFile);
        myFile->deleteLater();

        emit myFilesChanged();
        broadcastFiles();
    }

    void Server::requestDownloadFile(Model::File *file)
    {
        qDebug() << "Server::downloadFile";
        for (Model::Connection *connection : connections)
        {
            if (connection->getFiles().contains(file))
            {
                QJsonObject json;
                json["type"] = "prepareUpload";
                json["id"] = file->getId();
                json["ip"] = "server";
                connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                break;
            }
        }
    }

    void Server::handleNewConnection()
    {
        qDebug() << "server new connection";
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::handleDisconnected);
        connections.push_back(new Model::Connection(socket));
        emit newConnection();

        broadcastFiles();
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
                else if (type == "downloadFile")
                {
                    const QString code = obj["id"].toString();
                    handleClientRequestDownloadFile(socket, code);
                }
                else if (type == "readyToUpload")
                {
                    const QString reveiverIp = obj.value("ip").toString();
                    const int port = obj.value("port").toInt();
                    const QString fileId = obj.value("id").toString();
                    handleClientReadyToUploadFile(socket->peerAddress().toString(), reveiverIp, port, fileId);
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
                auto it = std::remove_if(connections.begin(), connections.end(), [socket](Model::Connection *connection) {
                    return connection->getTcpSocket() == socket;
                });
                if (it != connections.end())
                {
                    connections.erase(it, connections.end());
                    (*it)->deleteLater();
                    emit connectionsChanged();
                    broadcastFiles();
                }
            }
        }
    }
} // namespace Network
