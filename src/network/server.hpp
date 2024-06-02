#pragma once

#include <QObject>
#include <QList>
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "models/connection.hpp"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
class QWebSocketServer;
class QWebSocket;
QT_END_NAMESPACE

namespace Network
{
    class WebServer;

    class Server : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Status tcpStatus READ getTcpStatus NOTIFY tcpStatusChanged)
        Q_PROPERTY(Status wsStatus READ getWsStatus NOTIFY wsStatusChanged)
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
        Q_INVOKABLE void startWs(const QString &address);
        Q_INVOKABLE void stop();
        Q_INVOKABLE void stopWs();
        const QList<Model::MyFile *> &getMyFiles();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *myFile);
        Q_INVOKABLE void requestDownloadFile(Model::File *file);
        QList<Model::Connection *> getConnections() const;

    signals:
        void tcpStatusChanged() const;
        void wsStatusChanged() const;
        void connectionsChanged() const;
        void myFilesChanged() const;
        void wsInfoChanged(int wsPort, int weServerPort) const;

    private:
        Status getTcpStatus() const;
        Status getWsStatus() const;
        void broadcastFiles() const;
        void addClientFiles(QTcpSocket *socket, const QJsonArray &filesArray);
        void addWsClientFiles(QWebSocket *socket, const QJsonArray &filesArray);
        void handleClientRequestDownloadFile(QTcpSocket *clientSocket, const QString &id);
        void handleWsClientRequestDownloadFile(QWebSocket *clientSocket, const QString &id);
        void handleClientReadyToUploadFile(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId);
        void handleClientReadyToUploadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId);
        void handleClientReadyToDownloadFileForWeb(const QString &senderIp, const QString &reveiverIp, int port, const QString &fileId);

    private slots:
        void handleTcpNewConnection();
        void handleTcpReadyRead();
        void handleTcpDisconnected();
        void handleWsNewConnection();
        void handleWsTextMessageReceived(const QString &message);
        void handleWsDisconnected();

    private:
        Status tcpStatus;
        Status wsStatus;
        QTcpServer *tcpServer;
        QWebSocketServer *wsServer;
        WebServer *webServer;
        QList<Model::MyFile *> myFiles;
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

    inline Server::Status Server::getTcpStatus() const
    {
        return tcpStatus;
    }

    inline Server::Status Server::getWsStatus() const
    {
        return wsStatus;
    }
} // namespace Network
