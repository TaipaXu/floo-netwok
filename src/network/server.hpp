#pragma once

#include <QObject>
#include <QList>
#include "models/connection.hpp"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
class QWebSocketServer;
class QWebSocket;
QT_END_NAMESPACE

namespace Model
{
    class File;
    class MyFile;
} // namespace Model

namespace Network
{
    class WebServer;

    class Server : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Status tcpStatus READ getTcpStatus NOTIFY tcpStatusChanged)
        Q_PROPERTY(Status wsStatus READ getWSStatus NOTIFY wsStatusChanged)
        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)
        Q_PROPERTY(QList<Model::MyFile *> myFiles READ getMyFiles NOTIFY myFilesChanged)

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

        Q_INVOKABLE bool startTCP(const QString &address, int port);
        Q_INVOKABLE void stopTCP();
        Q_INVOKABLE void startWS(const QString &address);
        Q_INVOKABLE void stopWS();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *const myFile);
        Q_INVOKABLE void requestDownloadFile(const Model::File *const file) const;

    signals:
        void tcpStatusChanged() const;
        void wsStatusChanged() const;
        void wsInfoChanged(int wsPort, int weServerPort) const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        Status getTcpStatus() const;
        Status getWSStatus() const;
        const QList<Model::MyFile *> &getMyFiles() const;
        QList<Model::Connection *> getConnections() const;
        void broadcastFiles() const;
        void addClientFiles(const QTcpSocket *const socket, const QJsonArray &filesArray) const;
        void addWSClientFiles(const QWebSocket *const socket, const QJsonArray &filesArray) const;
        void handleClientRequestDownloadFile(QTcpSocket *const clientSocket, const QString &id) const;
        void handleWSClientRequestDownloadFile(QWebSocket *const clientSocket, const QString &id) const;
        void handleClientReadyToUploadFile(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId) const;
        void handleClientReadyToUploadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId) const;
        void handleClientReadyToDownloadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId) const;

    private slots:
        void handleTcpNewConnection();
        void handleTcpReadyRead() const;
        void handleTcpDisconnected();
        void handleWSNewConnection();
        void handleWSTextMessageReceived(const QString &message) const;
        void handleWSDisconnected();

    private:
        Status tcpStatus;
        QTcpServer *tcpServer;
        QWebSocketServer *wsServer;
        Status wsStatus;
        WebServer *webServer;
        QList<Model::MyFile *> myFiles;
        QList<Model::Connection *> connections;
    };

    inline Server::Status Server::getTcpStatus() const
    {
        return tcpStatus;
    }

    inline Server::Status Server::getWSStatus() const
    {
        return wsStatus;
    }

    inline QList<Model::Connection *> Server::getConnections() const
    {
        return connections;
    }

    inline const QList<Model::MyFile *> &Server::getMyFiles() const
    {
        return myFiles;
    }
} // namespace Network
