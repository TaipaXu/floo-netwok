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
#include "network/sendManager.hpp"
#include "network/receiveManager.hpp"
#include "network/webServer.hpp"

namespace Network
{
    Server::Server(QObject *parent)
        : QObject(parent), tcpStatus{Status::Unconnected}, wsStatus{Status::Unconnected}, tcpServer{nullptr}, wsServer{nullptr}, webServer{nullptr}
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
        tcpStatus = Status::Connecting;
        emit tcpStatusChanged();
        if (!tcpServer)
        {
            tcpServer = new QTcpServer(this);
            connect(tcpServer, &QTcpServer::newConnection, this, &Server::handleTcpNewConnection);
            connect(tcpServer, &QTcpServer::acceptError, [this](QAbstractSocket::SocketError socketError) {
                qDebug() << "server accept error" << socketError;
            });
        }

        bool result = tcpServer->listen(QHostAddress(address), port);
        if (result)
        {
            qDebug() << "Server started at" << address << port;
            tcpStatus = Status::Connected;
        }
        else
        {
            qDebug() << "Server start failed";
            tcpStatus = Status::Unconnected;
        }
        emit tcpStatusChanged();
        return result;
    }

    void Server::startWs(const QString &address)
    {
        qDebug() << "start ws" << address;
        wsStatus = Status::Connecting;
        emit wsStatusChanged();
        if (!wsServer)
        {
            wsServer = new QWebSocketServer("Server", QWebSocketServer::NonSecureMode, this);
            connect(wsServer, &QWebSocketServer::newConnection, this, &Server::handleWsNewConnection);
            connect(wsServer, &QWebSocketServer::serverError, [this](QWebSocketProtocol::CloseCode closeCode) {
                qDebug() << "ws server error" << closeCode;
            });
        }

        int port = 1000;
        while (!wsServer->listen(QHostAddress(address), port))
        {
            port++;
        }
        qDebug() << "port" << port;
        wsStatus = Status::Connected;
        emit wsStatusChanged();

        webServer = new WebServer();
        int webServerPort = webServer->start(address);
        qDebug() << "webServerPort" << webServerPort;

        emit wsInfoChanged(port, webServerPort);
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

        tcpStatus = Status::Unconnected;
        emit tcpStatusChanged();
        emit connectionsChanged();
        broadcastFiles();
    }

    void Server::stopWs()
    {
        if (wsServer)
        {
            wsServer->deleteLater();
            wsServer = nullptr;
        }

        for (auto &&connection : connections)
        {
            if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
            {
                connection->deleteLater();
            }
            connections.removeOne(connection);
        }

        wsStatus = Status::Unconnected;
        emit wsStatusChanged();
        emit connectionsChanged();
        broadcastFiles();

        if (webServer)
        {
            webServer->deleteLater();
            webServer = nullptr;
        }
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

            if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
            {
                json["you"] = connection->getAddress();
            }
        }
        json["files"] = ipFiles;

        const QJsonDocument doc(json);
        const QByteArray data = doc.toJson(QJsonDocument::Compact);
        for (Model::Connection *connection : connections)
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
                const int size = fileObj.value("size").toInteger();
                files.push_back(new Model::File(id, name, size, connection));
            }
            connection->setFiles(files);
            emit connectionsChanged();
        }

        broadcastFiles();
    }

    void Server::addWsClientFiles(QWebSocket *socket, const QJsonArray &filesArray)
    {
        qDebug() << "add ws client files";
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
                    json["id"] = id;
                    json["ip"] = connection->getAddress();
                    if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
                    {
                        json["type"] = "prepareUpload";
                    }
                    else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
                    {
                        json["type"] = "prepareDownloadForWeb";
                    }
                    clientSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                    break;
                }
            }
        }
    }

    void Server::handleWsClientRequestDownloadFile(QWebSocket *clientSocket, const QString &id)
    {
        qDebug() << "handle ws client request download file";
        for (auto &&file : myFiles)
        {
            if (file->getId() == id)
            {
                const int port = Network::SendManager::getInstance()->createHttpSender(file->getPath());
                QJsonObject json;
                json["type"] = "uploadFileReady";
                json["id"] = id;
                json["ip"] = wsServer->serverAddress().toString();
                json["port"] = port;
                clientSocket->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
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
                    json["type"] = "prepareUploadForWeb";
                    json["id"] = id;
                    json["ip"] = clientSocket->peerAddress().toString();
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
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

    void Server::handleClientReadyToUploadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId)
    {
        qDebug() << "handle client ready to upload file for web";
        qDebug() << "senderIp" << senderIp << "reveiverIp" << reveiverIp << "port" << port << "fileId" << fileId;

        for (Model::Connection *connection : connections)
        {
            if (connection->getAddress() == reveiverIp)
            {
                QJsonObject json;
                json["type"] = "uploadFileReady";
                json["id"] = fileId;
                json["ip"] = senderIp;
                json["port"] = port;
                connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                break;
            }
        }
    }

    void Server::handleClientReadyToDownloadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId)
    {
        qDebug() << "handle client ready to download file for web";
        qDebug() << "senderIp" << senderIp << "reveiverIp" << reveiverIp << "port" << port << "fileId" << fileId;

        for (Model::Connection *connection : connections)
        {
            if (connection->getAddress() == senderIp)
            {
                QJsonObject json;
                json["type"] = "prepareUpload";
                json["id"] = fileId;
                json["ip"] = reveiverIp;
                json["port"] = port;
                connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                break;
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
                if (connection->getLinkType() == Model::Connection::LinkType::TcpSocket)
                {
                    connection->getTcpSocket()->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
                }
                else if (connection->getLinkType() == Model::Connection::LinkType::WSSocket)
                {
                    const int port = ReceiveManager::getInstance()->createHttpReceiver();
                    json["port"] = port;
                    connection->getWsSocket()->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));
                }
                break;
            }
        }
    }

    void Server::handleTcpNewConnection()
    {
        qDebug() << "server new connection";
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleTcpReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::handleTcpDisconnected);
        connections.push_back(new Model::Connection(socket));
        emit connectionsChanged();

        broadcastFiles();
    }

    void Server::handleTcpReadyRead()
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
        qDebug() << "server disconnected";

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

    void Server::handleWsNewConnection()
    {
        qDebug() << "ws new connection";
        QWebSocket *const socket = wsServer->nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &Server::handleWsTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, this, &Server::handleWsDisconnected);
        connections.push_back(new Model::Connection(socket));
        emit connectionsChanged();

        broadcastFiles();
    }

    void Server::handleWsTextMessageReceived(const QString &message)
    {
        qDebug() << "ws received message" << message;
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
                    addWsClientFiles(socket, filesArray);
                }
                else if (type == "downloadFile")
                {
                    const QString code = obj["id"].toString();
                    handleWsClientRequestDownloadFile(socket, code);
                }
            }
        }
    }

    void Server::handleWsDisconnected()
    {
        qDebug() << "ws disconnected";
        if (wsServer)
        {
            QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
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
