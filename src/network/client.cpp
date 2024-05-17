#include "./client.hpp"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "utils/utils.hpp"

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
                    const QString id = fileObj.value("code").toString();
                    const QString name = fileObj.value("name").toString();
                    const int size = fileObj.value("path").toInteger();
                    files.push_back(new Model::File(id, name, size, connection));
                }
                connection->setFiles(files);

                emit connectionsChanged();
            }
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
        }
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
