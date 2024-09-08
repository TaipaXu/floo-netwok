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

    void Client::requestDownloadFile(const Model::File *const file)
    {
        const QString fileId = file->getId();
        if (requestingFiles.contains(fileId))
        {
            requestingFiles[fileId]++;
        }
        else
        {
            requestingFiles[fileId] = 1;
        }
        QJsonObject json{
            {"type", "downloadFile"},
            {"fileId", fileId},
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
                    const QString id = fileObject["id"].toString();
                    const QString name = fileObject["name"].toString();
                    const int size = fileObject["size"].toInteger();
                    files.push_back(new Model::File(id, name, size, connection));
                }
                connection->setFiles(files);

                emit connectionsChanged();
            }
        }
    }

    void Client::handleRequestPrepareUploadFile(const QString &recordId, const QString &fileId) const
    {
        auto file = std::find_if(myFiles.begin(), myFiles.end(), [&fileId](const Model::MyFile *const myFile) {
            return myFile->getId() == fileId;
        });
        if (file != myFiles.end())
        {
            GET_SEND_MANAGER_INSTANCE
            const int port = sendManager->createTcpSender((*file)->getPath());
            QJsonObject json{
                {"type", "readyToUpload"},
                {"recordId", recordId},
                {"port", port},
            };
            tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
    }

    void Client::handleRequestPrepareUploadFileForWeb(const QString &recordId, const QString &fileId) const
    {
        auto file = std::find_if(myFiles.begin(), myFiles.end(), [&fileId](const Model::MyFile *const myFile) {
            return myFile->getId() == fileId;
        });
        if (file != myFiles.end())
        {
            GET_SEND_MANAGER_INSTANCE
            auto [downloadId, port] = sendManager->createHttpSender((*file)->getPath());
            QJsonObject json{
                {"type", "readyToUploadForWeb"},
                {"recordId", recordId},
                {"downloadId", downloadId},
                {"port", port},
            };
            tcpSocket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        }
    }

    void Client::handleRequestUploadFileReady(const QString &ip, int port, const QString &fileId)
    {
        if (requestingFiles.contains(fileId))
        {
            requestingFiles[fileId]--;
            if (requestingFiles[fileId] == 0)
            {
                requestingFiles.erase(fileId);
            }

            GET_RECEIVE_MANAGER_INSTANCE
            receiveManager->createTcpReceiver(ip, port);
        }
    }

    void Client::handleRequestPrepareDownloadFileFromWeb(const QString &recordId) const
    {
        GET_RECEIVE_MANAGER_INSTANCE
        auto [downloadId, port] = receiveManager->createHttpReceiver();
        QJsonObject json{
            {"type", "readyToDownloadFromWeb"},
            {"recordId", recordId},
            {"downloadId", downloadId},
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
                const QString recordId = obj["recordId"].toString();
                const QString fileId = obj["fileId"].toString();
                handleRequestPrepareUploadFile(recordId, fileId);
            }
            else if (type == "prepareUploadForWeb")
            {
                const QString recordId = obj["recordId"].toString();
                const QString fileId = obj["fileId"].toString();
                handleRequestPrepareUploadFileForWeb(recordId, fileId);
            }
            else if (type == "uploadFileReady")
            {
                const QString senderIp = obj["ip"].toString();
                const int port = obj["port"].toInt();
                const QString fileId = obj["fileId"].toString();
                handleRequestUploadFileReady(senderIp, port, fileId);
            }
            else if (type == "prepareDownloadFromWeb")
            {
                const QString recordId = obj["recordId"].toString();
                handleRequestPrepareDownloadFileFromWeb(recordId);
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
