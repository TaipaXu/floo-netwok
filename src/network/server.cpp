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
        stopTcp();
        stopWs();
    }

    bool Server::startTcp(const QString &address, int port)
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

    void Server::stopTcp()
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

    void Server::startWs(const QString &address)
    {
        wsStatus = Status::Connecting;
        emit wsStatusChanged();
        if (!wsServer)
        {
            wsServer = new QWebSocketServer("Server", QWebSocketServer::NonSecureMode, this);
            connect(wsServer, &QWebSocketServer::newConnection, this, &Server::handleWsNewConnection);
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

    void Server::stopWs()
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
                if (fileInfo.exists() && fileInfo.isFile())
                {
                    this->myFiles.append(new Model::MyFile(fileInfo.fileName(), fileInfo.size(), fileInfo.filePath(), this));
                    if (!appended)
                    {
                        appended = true;
                    }
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

    void Server::requestDownloadFile(const Model::File *const file)
    {
        for (auto &&connection : connections)
        {
            if (connection->getFiles().contains(file))
            {
                if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
                {
                    Record record{
                        .fileId = file->getId(),
                        .senderIp = connection->getAddress(),
                        .receiverIp = "server",
                    };
                    records.push_back(record);
                    QJsonObject json{
                        {"type", "prepareUpload"},
                        {"recordId", record.id},
                        {"fileId", record.fileId},
                    };
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                }
                else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
                {
                    GET_RECEIVE_MANAGER_INSTANCE
                    auto [downloadId, port] = receiveManager->createHttpReceiver();
                    QJsonObject json{
                        {"type", "prepareUpload"},
                        {"fileId", file->getId()},
                        {"ip", "server"},
                        {"downloadId", downloadId},
                        {"port", port},
                    };
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
                const QString id = fileObj["id"].toString();
                const QString name = fileObj["name"].toString();
                const int size = fileObj["size"].toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
            broadcastFiles();
        }
    }

    void Server::addWsClientFiles(const QWebSocket *const socket, const QJsonArray &filesArray) const
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
                const QString id = fileObj["id"].toString();
                const QString name = fileObj["name"].toString();
                const int size = fileObj["size"].toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
            broadcastFiles();
        }
    }

    void Server::handleClientRequestDownloadFile(QTcpSocket *const clientSocket, const QString &fileId)
    {
        for (auto &&file : myFiles)
        {
            if (file->getId() == fileId)
            {
                GET_SEND_MANAGER_INSTANCE
                const int port = sendManager->createTcpSender(file->getPath());
                QJsonObject json{
                    {"type", "uploadFileReady"},
                    {"ip", tcpServer->serverAddress().toString()},
                    {"port", port},
                    {"fileId", fileId},
                };
                clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                return;
            }
        }

        for (auto &&connection : connections)
        {
            for (auto &&file : connection->getFiles())
            {
                if (file->getId() == fileId)
                {
                    Record record{
                        .fileId = fileId,
                        .senderIp = connection->getAddress(),
                        .receiverIp = clientSocket->peerAddress().toString(),
                    };
                    records.push_back(record);
                    QJsonObject json{
                        {"recordId", record.id},
                        {"fileId", fileId},
                    };
                    if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
                    {
                        json["type"] = "prepareUpload";
                    }
                    else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
                    {
                        json["type"] = "prepareDownloadFromWeb";
                    }
                    clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    return;
                }
            }
        }
    }

    void Server::handleWsClientRequestDownloadFile(QWebSocket *const clientSocket, const QString &fileId)
    {
        for (auto &&file : myFiles)
        {
            if (file->getId() == fileId)
            {
                GET_SEND_MANAGER_INSTANCE
                auto [downloadId, port] = sendManager->createHttpSender(file->getPath());
                QJsonObject json{
                    {"type", "uploadFileReady"},
                    {"ip", "server"},
                    {"downloadId", downloadId},
                    {"port", port},
                    {"fileId", fileId},
                };
                clientSocket->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                return;
            }
        }

        for (auto &&connection : connections)
        {
            for (auto &&file : connection->getFiles())
            {
                if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket && file->getId() == fileId)
                {
                    Record record{
                        .fileId = fileId,
                        .senderIp = connection->getAddress(),
                        .receiverIp = clientSocket->peerAddress().toString(),
                    };
                    records.push_back(record);
                    QJsonObject json{
                        {"type", "prepareUploadForWeb"},
                        {"recordId", record.id},
                        {"fileId", fileId},
                    };
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    return;
                }
            }
        }
    }

    void Server::handleClientReadyToUploadFile(const QString &recordId, int port)
    {
        auto record = std::find_if(records.begin(), records.end(), [&recordId](const Record &record) {
            return record.id == recordId;
        });
        if (record != records.end())
        {
            const QString reveiverIp = record->receiverIp;
            if (reveiverIp == "server")
            {
                GET_RECEIVE_MANAGER_INSTANCE
                receiveManager->createTcpReceiver(record->senderIp, port);
            }
            else
            {
                for (auto &&connection : connections)
                {
                    if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket && connection->getAddress() == reveiverIp)
                    {
                        QJsonObject json{
                            {"type", "uploadFileReady"},
                            {"ip", record->senderIp},
                            {"port", port},
                            {"fileId", record->fileId},
                        };
                        connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                        break;
                    }
                }
            }

            records.erase(record);
        }
    }

    void Server::handleClientReadyToUploadFileForWeb(const QString &recordId, const QString &downloadId, int port)
    {
        auto record = std::find_if(records.begin(), records.end(), [&recordId](const Record &record) {
            return record.id == recordId;
        });
        if (record != records.end())
        {
            for (auto &&connection : connections)
            {
                if (connection->getLinkType() == Model::Connection::LinkType::WSSocket && connection->getAddress() == record->receiverIp)
                {
                    QJsonObject json{
                        {"type", "uploadFileReady"},
                        {"ip", record->senderIp},
                        {"downloadId", downloadId},
                        {"port", port},
                        {"fileId", record->fileId},
                    };
                    connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    break;
                }
            }

            records.erase(record);
        }
    }

    void Server::handleClientReadyToDownloadFromWeb(const QString &recordId, const QString &downloadId, int port)
    {
        auto record = std::find_if(records.begin(), records.end(), [&recordId](const Record &record) {
            return record.id == recordId;
        });
        if (record != records.end())
        {
            for (auto &&connection : connections)
            {
                if (connection->getLinkType() == Model::Connection::LinkType::WSSocket && connection->getAddress() == record->senderIp)
                {
                    QJsonObject json{
                        {"type", "prepareUpload"},
                        {"fileId", record->fileId},
                        {"ip", record->receiverIp},
                        {"downloadId", downloadId},
                        {"port", port},
                    };
                    connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    break;
                }
            }

            records.erase(record);
        }
    }

    void Server::handleTcpNewConnection()
    {
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        auto it = std::find_if(connections.begin(), connections.end(), [socket](Model::Connection *connection) {
            return connection->getLinkType() == Model::Connection::LinkType::TcpSocket && connection->getAddress() == socket->peerAddress().toString();
        });
        if (it != connections.end())
        {
            socket->close();
        }
        else
        {
            connect(socket, &QTcpSocket::readyRead, this, &Server::handleTcpReadyRead);
            connect(socket, &QTcpSocket::disconnected, this, &Server::handleTcpDisconnected);
            connections.push_back(new Model::Connection(socket));
            emit connectionsChanged();
            broadcastFiles();
        }
    }

    void Server::handleTcpReadyRead()
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
                const QString type = obj["type"].toString();
                if (type == "files")
                {
                    const QJsonArray filesArray = obj["files"].toArray();
                    addClientFiles(socket, filesArray);
                }
                else if (type == "downloadFile")
                {
                    const QString fileId = obj["fileId"].toString();
                    handleClientRequestDownloadFile(socket, fileId);
                }
                else if (type == "readyToUpload")
                {
                    const QString recordId = obj["recordId"].toString();
                    const int port = obj["port"].toInt();
                    handleClientReadyToUploadFile(recordId, port);
                }
                else if (type == "readyToUploadForWeb")
                {
                    const QString recordId = obj["recordId"].toString();
                    const QString downloadId = obj["downloadId"].toString();
                    const int port = obj["port"].toInt();
                    handleClientReadyToUploadFileForWeb(recordId, downloadId, port);
                }
                else if (type == "readyToDownloadFromWeb")
                {
                    const QString recordId = obj["recordId"].toString();
                    const QString downloadId = obj["downloadId"].toString();
                    const int port = obj["port"].toInt();
                    handleClientReadyToDownloadFromWeb(recordId, downloadId, port);
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

    void Server::handleWsNewConnection()
    {
        QWebSocket *const socket = wsServer->nextPendingConnection();
        auto it = std::find_if(connections.begin(), connections.end(), [socket](Model::Connection *connection) {
            return connection->getLinkType() == Model::Connection::LinkType::WSSocket && connection->getAddress() == socket->peerAddress().toString();
        });
        if (it != connections.end())
        {
            socket->close();
        }
        else
        {
            connect(socket, &QWebSocket::textMessageReceived, this, &Server::handleWsTextMessageReceived);
            connect(socket, &QWebSocket::disconnected, this, &Server::handleWsDisconnected);
            connections.push_back(new Model::Connection(socket));
            emit connectionsChanged();
            broadcastFiles();
        }
    }

    void Server::handleWsTextMessageReceived(const QString &message)
    {
        QWebSocket *const socket = qobject_cast<QWebSocket *>(sender());
        if (socket)
        {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
            if (!json.isNull() && jsonError.error == QJsonParseError::NoError)
            {
                const QJsonObject obj = json.object();
                const QString type = obj["type"].toString();
                if (type == "files")
                {
                    const QJsonArray filesArray = obj["files"].toArray();
                    addWsClientFiles(socket, filesArray);
                }
                else if (type == "downloadFile")
                {
                    const QString fileId = obj["fileId"].toString();
                    handleWsClientRequestDownloadFile(socket, fileId);
                }
            }
        }
    }

    void Server::handleWsDisconnected()
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
