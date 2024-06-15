#include "./server.hpp"
#include <QTcpServer>
#include <QTcpSocket>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "network/sendManager.hpp"
#include "network/receiveManager.hpp"
#include "network/webServer.hpp"

namespace Network
{
    Server::Server(QObject *parent)
        : QObject(parent),
          tcpStatus{Status::Unconnected},
          tcpServer{nullptr},
          wsStatus{Status::Unconnected},
          wsServer{nullptr},
          webServer{nullptr}
    {
    }

    Server::~Server()
    {
        stopTCP();
        stopWS();
    }

    bool Server::startTCP(const QString &address, int port)
    {
        tcpStatus = Status::Connecting;
        emit tcpStatusChanged();
        if (!tcpServer)
        {
            tcpServer = new QTcpServer(this);
            connect(tcpServer, &QTcpServer::newConnection, this, &Server::handleTcpNewConnection);
        }
        bool result = tcpServer->listen(QHostAddress(address), port);
        if (result)
        {
            tcpStatus = Status::Connected;
        }
        else
        {
            tcpStatus = Status::Unconnected;
        }
        emit tcpStatusChanged();
        return result;
    }

    void Server::stopTCP()
    {
        if (tcpServer)
        {
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }
        tcpStatus = Status::Unconnected;
        emit tcpStatusChanged();

        for (auto &&connection : connections)
        {
            if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
            {
                connection->deleteLater();
            }
            connections.removeOne(connection);
        }
        emit connectionsChanged();
        broadcastFiles();
    }

    void Server::startWS(const QString &address)
    {
        wsStatus = Status::Connecting;
        emit wsStatusChanged();
        if (!wsServer)
        {
            wsServer = new QWebSocketServer("Server", QWebSocketServer::NonSecureMode, this);
            connect(wsServer, &QWebSocketServer::newConnection, this, &Server::handleWSNewConnection);
        }

        int port = 1024;
        while (!wsServer->listen(QHostAddress(address), port))
        {
            port++;
        }
        wsStatus = Status::Connected;
        emit wsStatusChanged();

        webServer = new WebServer();
        const int webServerPort = webServer->start(address);

        emit wsInfoChanged(port, webServerPort);
    }

    void Server::stopWS()
    {
        if (wsServer)
        {
            wsServer->deleteLater();
            wsServer = nullptr;
        }
        wsStatus = Status::Unconnected;
        emit wsStatusChanged();

        for (auto &&connection : connections)
        {
            if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
            {
                connection->deleteLater();
            }
            connections.removeOne(connection);
        }
        emit connectionsChanged();
        broadcastFiles();

        if (webServer)
        {
            webServer->deleteLater();
            webServer = nullptr;
        }
    }

    void Server::addMyFiles(const QList<QUrl> &myFiles)
    {
        bool appended = false;
        for (const QUrl &url : myFiles)
        {
            auto found = std::find_if(this->myFiles.begin(), this->myFiles.end(), [&url](const Model::MyFile *const myFile) {
                return myFile->getPath() == url.toLocalFile();
            });
            if (found == this->myFiles.end())
            {
                const QFileInfo fileInfo(url.toLocalFile());
                this->myFiles.append(new Model::MyFile(fileInfo.fileName(), fileInfo.size(), fileInfo.filePath(), this));
                if (!appended)
                {
                    appended = true;
                }
            }
        }

        if (appended)
        {
            emit myFilesChanged();
            broadcastFiles();
        }
    }

    void Server::removeMyFile(Model::MyFile *const myFile)
    {
        myFiles.removeOne(myFile);
        myFile->deleteLater();

        emit myFilesChanged();
        broadcastFiles();
    }

    void Server::requestDownloadFile(const Model::File *const file) const
    {
        for (auto &&connection : connections)
        {
            if (connection->getFiles().contains(file))
            {
                QJsonObject json{
                    {"type", "prepareUpload"},
                    {"id", file->getId()},
                    {"ip", "server"},
                };
                if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
                {
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                }
                else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
                {
                    GET_RECEIVE_MANAGER_INSTANCE
                    const int port = receiveManager->createHttpReceiver();
                    json["port"] = port;
                    connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                }
                break;
            }
        }
    }

    void Server::broadcastFiles() const
    {
        if (myFiles.empty() && connections.empty())
        {
            return;
        }

        QJsonObject json{
            {"type", "files"},
        };
        json["type"] = "files";
        QJsonObject ipFiles{
            {"server", Model::toJson(myFiles)},
        };
        for (auto &&connection : connections)
        {
            ipFiles[connection->getAddress()] = Model::toJson(connection->getFiles());

            if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
            {
                json["you"] = connection->getAddress();
            }
        }
        json["files"] = ipFiles;

        const QJsonDocument doc(json);
        const QByteArray data = doc.toJson(QJsonDocument::Compact);
        for (auto &&connection : connections)
        {
            if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
            {
                connection->getTcpSocket()->write(data);
            }
            else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
            {

                connection->getWsSocket()->sendTextMessage(data);
            }
        }
    }

    void Server::addClientFiles(const QTcpSocket *const socket, const QJsonArray &filesArray) const
    {
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
                const int size = fileObj.value("size").toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
            broadcastFiles();
        }
    }

    void Server::addWSClientFiles(const QWebSocket *const socket, const QJsonArray &filesArray) const
    {
        Model::Connection *connection = nullptr;
        for (Model::Connection *conn : connections)
        {
            if (conn->getWsSocket() == socket)
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
                const int size = fileObj.value("size").toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
            broadcastFiles();
        }
    }

    void Server::handleClientRequestDownloadFile(QTcpSocket *const clientSocket, const QString &id) const
    {
        for (auto &&file : myFiles)
        {
            if (file->getId() == id)
            {
                GET_SEND_MANAGER_INSTANCE
                const int port = sendManager->createSender(file->getPath());
                QJsonObject json{
                    {"type", "uploadFileReady"},
                    {"id", id},
                    {"ip", tcpServer->serverAddress().toString()},
                    {"port", port},
                };
                clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                return;
            }
        }

        for (auto &&connection : connections)
        {
            for (auto &&file : connection->getFiles())
            {
                if (file->getId() == id)
                {
                    QJsonObject json{
                        {"id", id},
                        {"ip", connection->getAddress()},
                    };
                    if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
                    {
                        json["type"] = "prepareUpload";
                    }
                    else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
                    {
                        json["type"] = "prepareDownloadForWeb";
                    }
                    clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    return;
                }
            }
        }
    }

    void Server::handleWSClientRequestDownloadFile(QWebSocket *const clientSocket, const QString &id) const
    {
        for (auto &&file : myFiles)
        {
            if (file->getId() == id)
            {
                GET_SEND_MANAGER_INSTANCE
                const int port = sendManager->createHttpSender(file->getPath());
                QJsonObject json{
                    {"type", "uploadFileReady"},
                    {"id", id},
                    {"ip", wsServer->serverAddress().toString()},
                    {"port", port},
                };
                clientSocket->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                return;
            }
        }

        for (auto &&connection : connections)
        {
            for (auto &&file : connection->getFiles())
            {
                if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket && file->getId() == id)
                {
                    QJsonObject json{
                        {"type", "prepareUploadForWeb"},
                        {"id", id},
                        {"ip", clientSocket->peerAddress().toString()},
                    };
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    return;
                }
            }
        }
    }

    void Server::handleClientReadyToUploadFile(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId) const
    {
        if (reveiverIp == "server")
        {
            GET_RECEIVE_MANAGER_INSTANCE
            receiveManager->createReceiver(senderIp, port);
        }
        else
        {
            for (auto &&connection : connections)
            {
                if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket && connection->getAddress() == reveiverIp)
                {
                    QJsonObject json{
                        {"type", "uploadFileReady"},
                        {"id", fileId},
                        {"ip", senderIp},
                        {"port", port},
                    };
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    break;
                }
            }
        }
    }

    void Server::handleClientReadyToUploadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId) const
    {
        for (auto &&connection : connections)
        {
            if (connection->getLinkType() == Model::Connection::LinkType::WSSocket && connection->getAddress() == reveiverIp)
            {
                QJsonObject json{
                    {"type", "uploadFileReady"},
                    {"id", fileId},
                    {"ip", senderIp},
                    {"port", port},
                };
                connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                break;
            }
        }
    }

    void Server::handleClientReadyToDownloadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId) const
    {
        for (auto &&connection : connections)
        {
            if (connection->getLinkType() == Model::Connection::LinkType::WSSocket && connection->getAddress() == senderIp)
            {
                QJsonObject json{
                    {"type", "prepareUpload"},
                    {"id", fileId},
                    {"ip", reveiverIp},
                    {"port", port},
                };
                connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                break;
            }
        }
    }

    void Server::handleTcpNewConnection()
    {
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleTcpReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::handleTcpDisconnected);
        connections.push_back(new Model::Connection(socket));
        emit connectionsChanged();
        broadcastFiles();
    }

    void Server::handleTcpReadyRead() const
    {
        QTcpSocket *const socket = qobject_cast<QTcpSocket *const>(sender());
        if (socket)
        {
            QByteArray data = socket->readAll();
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(data, &jsonError);
            if (!json.isNull() && jsonError.error == QJsonParseError::NoError)
            {
                const QJsonObject obj = json.object();
                const QString type = obj.value("type").toString();
                if (type == "files")
                {
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
                else if (type == "readyToUploadForWeb")
                {
                    const QString reveiverIp = obj.value("ip").toString();
                    const int port = obj.value("port").toInt();
                    const QString fileId = obj.value("id").toString();
                    handleClientReadyToUploadFileForWeb(socket->peerAddress().toString(), reveiverIp, port, fileId);
                }
                else if (type == "readyToDownloadForWeb")
                {
                    const QString senderIp = obj.value("ip").toString();
                    const int port = obj.value("port").toInt();
                    const QString fileId = obj.value("id").toString();
                    handleClientReadyToDownloadFileForWeb(senderIp, socket->peerAddress().toString(), port, fileId);
                }
            }
        }
    }

    void Server::handleTcpDisconnected()
    {
        if (tcpServer)
        {
            QTcpSocket *const socket = qobject_cast<QTcpSocket *>(sender());
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

    void Server::handleWSNewConnection()
    {
        QWebSocket *const socket = wsServer->nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &Server::handleWSTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, this, &Server::handleWSDisconnected);
        connections.push_back(new Model::Connection(socket));
        emit connectionsChanged();
        broadcastFiles();
    }

    void Server::handleWSTextMessageReceived(const QString &message) const
    {
        QWebSocket *const socket = qobject_cast<QWebSocket *>(sender());
        if (socket)
        {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
            if (!json.isNull() && jsonError.error == QJsonParseError::NoError)
            {
                const QJsonObject obj = json.object();
                const QString type = obj.value("type").toString();
                if (type == "files")
                {
                    const QJsonArray filesArray = obj.value("files").toArray();
                    addWSClientFiles(socket, filesArray);
                }
                else if (type == "downloadFile")
                {
                    const QString code = obj["id"].toString();
                    handleWSClientRequestDownloadFile(socket, code);
                }
            }
        }
    }

    void Server::handleWSDisconnected()
    {
        if (wsServer)
        {
            QWebSocket *const socket = qobject_cast<QWebSocket *>(sender());
            if (socket)
            {
                socket->deleteLater();
                auto it = std::remove_if(connections.begin(), connections.end(), [socket](Model::Connection *connection) {
                    return connection->getWsSocket() == socket;
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
