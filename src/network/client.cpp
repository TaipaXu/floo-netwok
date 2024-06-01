#include "./client.hpp"
#include <QTimer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include "utils/utils.hpp"
#include "network/sendManager.hpp"
#include "network/receiveManager.hpp"

namespace Network
{
    Client::Client(QObject *parent)
        : QObject(parent), status{Status::Unconnected}, tcpSocket{nullptr}
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
        status = Status::Connecting;
        emit statusChanged();
    }

    void Client::stop()
    {
        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }

        for (auto &&connection : connections)
        {
            connection->deleteLater();
        }
        connections.clear();
    }

    void Client::addMyFiles(const QList<QUrl> &myFiles)
    {
        qDebug() << "Client::addMyFiles";
        for (const QUrl &url : myFiles)
        {
            QFileInfo fileInfo(url.toLocalFile());
            this->myFiles.append(new Model::MyFile(fileInfo.fileName(), fileInfo.size(), fileInfo.filePath(), this));
        }
        qDebug() << "size: " << myFiles.size();

        emit myFilesChanged();
        sendFilesInfoToServer();
    }

    void Client::removeMyFile(Model::MyFile *myFile)
    {
        myFiles.removeOne(myFile);
        myFile->deleteLater();

        emit myFilesChanged();
        sendFilesInfoToServer();
    }

    void Client::requestDownloadFile(Model::File *file)
    {
        qDebug() << "requestDownloadFile";
        QJsonObject json;
        json["type"] = "downloadFile";
        json["id"] = file->getId();
        tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }

    void Client::sendFilesInfoToServer() const
    {
        if (!tcpSocket)
        {
            return;
        }

        QJsonObject json;
        QJsonArray filesArray = Model::toJson(myFiles);
        json["type"] = "files";
        json["files"] = filesArray;
        tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }

    void Client::handleRequestFiles(const QJsonObject &ipFiles)
    {
        qDebug() << "client request files" << ipFiles;
        for (auto &&i : ipFiles.keys())
        {
            if (!Utils::isLocalAddress(i))
            {
                Model::Connection *connection = nullptr;
                for (Model::Connection *conn : connections)
                {
                    if (conn->getAddress() == i)
                    {
                        connection = conn;
                        break;
                    }
                }
                if (!connection)
                {
                    connection = new Model::Connection(i);
                    connections.push_back(connection);
                }

                QList<Model::File *> files;
                const QJsonArray filesArray = ipFiles[i].toArray();
                for (auto &&j : filesArray)
                {
                    const QJsonObject fileObj = j.toObject();
                    const QString id = fileObj.value("id").toString();
                    const QString name = fileObj.value("name").toString();
                    const int size = fileObj.value("size").toInteger();
                    files.push_back(new Model::File(id, name, size, connection));
                }
                connection->setFiles(files);

                emit connectionsChanged();
            }
        }
    }

    void Client::handleRequestPrepareUploadFile(const QString &fileId, const QString &reveiverIp) const
    {
        qDebug() << "client request prepare upload file" << fileId << reveiverIp;
        auto file = std::find_if(myFiles.begin(), myFiles.end(), [fileId](Model::MyFile *myFile) {
            return myFile->getId() == fileId;
        });
        if (file != myFiles.end())
        {
            const int port = SendManager::getInstance()->createSender((*file)->getPath());
            QJsonObject json;
            json["type"] = "readyToUpload";
            json["ip"] = reveiverIp;
            json["port"] = port;
            json["id"] = fileId;
            tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
    }

    void Client::handleRequestPrepareUploadFileForWeb(const QString &fileId, const QString &reveiverIp) const
    {
        qDebug() << "client request prepare upload file for web" << fileId << reveiverIp;
        auto file = std::find_if(myFiles.begin(), myFiles.end(), [fileId](Model::MyFile *myFile) {
            return myFile->getId() == fileId;
        });
        if (file != myFiles.end())
        {
            const int port = SendManager::getInstance()->createHttpSender((*file)->getPath());
            QJsonObject json;
            json["type"] = "readyToUploadForWeb";
            json["ip"] = reveiverIp;
            json["port"] = port;
            json["id"] = fileId;
            tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
    }

    void Client::handleRequestUploadFileReady(const QString &ip, int port) const
    {
        qDebug() << "client request upload file ready" << ip << port;
        ReceiveManager::getInstance()->createReceiver(ip, port);
    }

    void Client::handleConnected()
    {
        qDebug() << "client connected";
        status = Status::Connected;
        emit statusChanged();
        emit connected();

        sendFilesInfoToServer();
    }

    void Client::handleReadyRead()
    {
        const QByteArray message = tcpSocket->readAll();
        qDebug() << "client received message" << message;
        QJsonParseError jsonError;
        const QJsonDocument json = QJsonDocument::fromJson(message, &jsonError);
        if (!json.isNull() && jsonError.error == QJsonParseError::NoError)
        {
            const QJsonObject obj = json.object();
            const QString type = obj["type"].toString();
            if (type == "files")
            {
                const QJsonObject ipFiles = obj["files"].toObject();
                handleRequestFiles(ipFiles);
            }
            else if (type == "prepareUpload")
            {
                const QString fileId = obj["id"].toString();
                const QString reveiverIp = obj["ip"].toString();
                handleRequestPrepareUploadFile(fileId, reveiverIp);
            }
            else if (type == "prepareUploadForWeb")
            {
                const QString fileId = obj["id"].toString();
                const QString reveiverIp = obj["ip"].toString();
                handleRequestPrepareUploadFileForWeb(fileId, reveiverIp);
            }
            else if (type == "uploadFileReady")
            {
                const QString senderIp = obj["ip"].toString();
                const int port = obj["port"].toInt();
                handleRequestUploadFileReady(senderIp, port);
            }
        }
    }

    void Client::handleError(QAbstractSocket::SocketError socketError)
    {
        qDebug() << "client error" << socketError;
        status = Status::Unconnected;
        emit statusChanged();
        emit connectError();
    }

    void Client::handleDisconnected()
    {
        qDebug() << "client disconnected";
        status = Status::Unconnected;
        emit statusChanged();
        emit disconnected();

        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }

        for (auto &&connection : connections)
        {
            connection->deleteLater();
        }
        connections.clear();
        emit connectionsChanged();
    }
} // namespace Network
