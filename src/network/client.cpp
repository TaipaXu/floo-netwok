#include "./client.hpp"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include "utils/utils.hpp"
#include "network/sendManager.hpp"
#include "network/receiveManager.hpp"

#include "models/file.hpp"
#include "models/myFile.hpp"

namespace Network
{
    Client::Client(QObject *parent)
        : QObject(parent),
          status{Status::Unconnected},
          tcpSocket{nullptr}
    {
    }

    Client::~Client()
    {
        stop();
    }

    void Client::start(const QString &address, int port)
    {
        status = Status::Connecting;
        emit statusChanged();
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

        for (auto &&connection : connections)
        {
            connection->deleteLater();
        }
        connections.clear();
        emit connectionsChanged();
    }

    void Client::addMyFiles(const QList<QUrl> &myFiles)
    {
        bool appended = false;
        for (const QUrl &url : myFiles)
        {
            auto found = std::find_if(this->myFiles.begin(), this->myFiles.end(), [&url](const Model::MyFile *const myFile) {
                return myFile->getPath() == url.toLocalFile();
            });
            if (found == this->myFiles.end())
            {
                QFileInfo fileInfo(url.toLocalFile());
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
            sendFilesInfoToServer();
        }
    }

    void Client::removeMyFile(Model::MyFile *const myFile)
    {
        myFiles.removeOne(myFile);
        myFile->deleteLater();

        emit myFilesChanged();
        sendFilesInfoToServer();
    }

    void Client::requestDownloadFile(const Model::File *const file) const
    {
        QJsonObject json{
            {"type", "downloadFile"},
            {"id", file->getId()},
        };
        tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }

    void Client::sendFilesInfoToServer() const
    {
        if (!tcpSocket || myFiles.empty())
        {
            return;
        }

        QJsonArray filesArray = Model::toJson(myFiles);
        QJsonObject json{
            {"type", "files"},
            {"files", filesArray},
        };
        tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }

    void Client::handleRequestFiles(const QJsonObject &ipFiles)
    {
        for (auto &&ip : ipFiles.keys())
        {
            if (!Utils::isLocalAddress(ip))
            {
                Model::Connection *connection = nullptr;
                for (Model::Connection *conn : connections)
                {
                    if (conn->getAddress() == ip)
                    {
                        connection = conn;
                        break;
                    }
                }
                if (!connection)
                {
                    connection = new Model::Connection(ip);
                    connections.push_back(connection);
                }

                QList<Model::File *> files;
                const QJsonArray filesArray = ipFiles[ip].toArray();
                for (auto &&fileJson : filesArray)
                {
                    const QJsonObject fileObject = fileJson.toObject();
                    const QString id = fileObject.value("id").toString();
                    const QString name = fileObject.value("name").toString();
                    const int size = fileObject.value("size").toInteger();
                    files.push_back(new Model::File(id, name, size, connection));
                }
                connection->setFiles(files);

                emit connectionsChanged();
            }
        }
    }

    void Client::handleRequestPrepareUploadFile(const QString &fileId, const QString &reveiverIp) const
    {
        auto file = std::find_if(myFiles.begin(), myFiles.end(), [&fileId](const Model::MyFile *const myFile) {
            return myFile->getId() == fileId;
        });
        if (file != myFiles.end())
        {
            GET_SEND_MANAGER_INSTANCE
            const int port = sendManager->createSender((*file)->getPath());
            QJsonObject json{
                {"type", "readyToUpload"},
                {"ip", reveiverIp},
                {"port", port},
                {"id", fileId},
            };
            tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
    }

    void Client::handleRequestPrepareUploadFileForWeb(const QString &fileId, const QString &reveiverIp) const
    {
        auto file = std::find_if(myFiles.begin(), myFiles.end(), [&fileId](const Model::MyFile *const myFile) {
            return myFile->getId() == fileId;
        });
        if (file != myFiles.end())
        {
            GET_SEND_MANAGER_INSTANCE
            const int port = sendManager->createHttpSender((*file)->getPath());
            QJsonObject json{
                {"type", "readyToUploadForWeb"},
                {"ip", reveiverIp},
                {"port", port},
                {"id", fileId},
            };
            tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
    }

    void Client::handleRequestUploadFileReady(const QString &ip, int port) const
    {
        GET_RECEIVE_MANAGER_INSTANCE
        receiveManager->createReceiver(ip, port);
    }

    void Client::handleRequestPrepareDownloadFileForWeb(const QString &fileId, const QString &senderIp) const
    {
        GET_RECEIVE_MANAGER_INSTANCE
        const int port = receiveManager->createHttpReceiver();
        QJsonObject json{
            {"type", "readyToDownloadForWeb"},
            {"id", fileId},
            {"ip", senderIp},
            {"port", port},
        };
        tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }

    void Client::handleConnected()
    {
        status = Status::Connected;
        emit statusChanged();
        sendFilesInfoToServer();
    }

    void Client::handleReadyRead()
    {
        const QByteArray message = tcpSocket->readAll();
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
            else if (type == "prepareDownloadForWeb")
            {
                const QString fileId = obj["id"].toString();
                const QString senderIp = obj["ip"].toString();
                handleRequestPrepareDownloadFileForWeb(fileId, senderIp);
            }
        }
    }

    void Client::handleError(QAbstractSocket::SocketError socketError)
    {
        status = Status::Unconnected;
        emit statusChanged();
        emit connectError();
    }

    void Client::handleDisconnected()
    {

        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }
        status = Status::Unconnected;
        emit statusChanged();

        for (auto &&connection : connections)
        {
            connection->deleteLater();
        }
        connections.clear();
        emit connectionsChanged();
    }
} // namespace Network
