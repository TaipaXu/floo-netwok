#pragma once

#include <QObject>
#include <QList>
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "models/connection.hpp"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
QT_END_NAMESPACE

namespace Network
{
    class Server : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
        Q_PROPERTY(QList<Model::MyFile *> myFiles READ getMyFiles NOTIFY myFilesChanged)
        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)

    public:
        enum class Status
        {
            Unconnected,
            Connected,
            Connecting,
        };
        Q_ENUM(Status)

    public:
        Server(QObject *parent = nullptr);
        ~Server();

        Q_INVOKABLE bool start(const QString &address, int port);
        Q_INVOKABLE void stop();
        const QList<Model::MyFile *> &getMyFiles();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *myFile);
        Q_INVOKABLE void requestDownloadFile(Model::File *file);
        QList<Model::Connection *> getConnections() const;

    signals:
        void statusChanged() const;
        void newConnection() const;
        void disconnected() const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        Status getStatus() const;
        void broadcastFiles() const;
        void addClientFiles(QTcpSocket *socket, const QJsonArray &filesArray);
        void handleClientRequestDownloadFile(QTcpSocket *clientSocket, const QString &id);
        void handleClientReadyToUploadFile(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId);

    private slots:
        void handleNewConnection();
        void handleReadyRead();
        void handleDisconnected();

    private:
        Status status;
        QList<Model::MyFile *> myFiles;
        QTcpServer *tcpServer;
        QList<Model::Connection *> connections;
    };

    inline const QList<Model::MyFile *> &Server::getMyFiles()
    {
        return myFiles;
    }

    inline QList<Model::Connection *> Server::getConnections() const
    {
        return connections;
    }

    inline Server::Status Server::getStatus() const
    {
        return status;
    }
} // namespace Network
