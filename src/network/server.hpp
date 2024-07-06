#pragma once

#include <list>
#include <QObject>
#include <QList>
#include <QUuid>
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
        Q_PROPERTY(Status wsStatus READ getWsStatus NOTIFY wsStatusChanged)
        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)
        Q_PROPERTY(QList<Model::MyFile *> myFiles READ getMyFiles NOTIFY myFilesChanged)

    private:
        struct Record
        {
            QString id = QUuid().toString();
            QString fileId;
            QString senderIp;
            QString receiverIp;
        };

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

        Q_INVOKABLE bool startTcp(const QString &address, int port);
        Q_INVOKABLE void stopTcp();
        Q_INVOKABLE void startWs(const QString &address);
        Q_INVOKABLE void stopWs();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *const myFile);
        Q_INVOKABLE void requestDownloadFile(const Model::File *const file);

    signals:
        void tcpStatusChanged() const;
        void wsStatusChanged() const;
        void wsInfoChanged(int wsPort, int weServerPort) const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        Status getTcpStatus() const;
        Status getWsStatus() const;
        const QList<Model::MyFile *> &getMyFiles() const;
        QList<Model::Connection *> getConnections() const;
        void broadcastFiles() const;
        void addClientFiles(const QTcpSocket *const socket, const QJsonArray &filesArray) const;
        void addWsClientFiles(const QWebSocket *const socket, const QJsonArray &filesArray) const;
        void handleClientRequestDownloadFile(QTcpSocket *const clientSocket, const QString &fileId);
        void handleWsClientRequestDownloadFile(QWebSocket *const clientSocket, const QString &fileId) const;
        void handleClientReadyToUploadFile(const QString &recordId, int port);
        void handleClientReadyToUploadFileForWeb(const QString &recordId, int port);
        void handleClientReadyToDownloadFromWeb(const QString &recordId, int port);

    private slots:
        void handleTcpNewConnection();
        void handleTcpReadyRead();
        void handleTcpDisconnected();
        void handleWsNewConnection();
        void handleWsTextMessageReceived(const QString &message) const;
        void handleWsDisconnected();

    private:
        Status tcpStatus;
        QTcpServer *tcpServer;
        QWebSocketServer *wsServer;
        Status wsStatus;
        WebServer *webServer;
        QList<Model::MyFile *> myFiles;
        QList<Model::Connection *> connections;
        std::list<Record> records;
    };

    inline Server::Status Server::getTcpStatus() const
    {
        return tcpStatus;
    }

    inline Server::Status Server::getWsStatus() const
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
