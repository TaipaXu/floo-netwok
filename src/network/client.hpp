#pragma once

#include <QObject>
#include <QAbstractSocket>
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "models/connection.hpp"

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

namespace Network
{
    class Client : public QObject
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
        Client(QObject *parent = nullptr);
        ~Client();

        Q_INVOKABLE void start(const QString &address, int port);
        Q_INVOKABLE void stop();
        const QList<Model::MyFile *> &getMyFiles();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *myFile);
        Q_INVOKABLE void requestDownloadFile(Model::File *file);
        QList<Model::Connection *> getConnections() const;

    signals:
        void statusChanged() const;
        void connected() const;
        void disconnected() const;
        void connectError() const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        Status getStatus() const;
        void sendFilesInfoToServer() const;
        void handleRequestFiles(const QJsonObject &ipFiles);
        void handleRequestPrepareUploadFile(const QString &fileId, const QString &reveiverIp) const;
        void handleRequestPrepareUploadFileForWeb(const QString &fileId, const QString &reveiverIp) const;
        void handleRequestUploadFileReady(const QString &ip, int port) const;
        void handleRequestPrepareDownloadFileForWeb(const QString &fileId, const QString &senderIp) const;

    private slots:
        void handleConnected();
        void handleReadyRead();
        void handleError(QAbstractSocket::SocketError socketError);
        void handleDisconnected();

    private:
        Status status;
        QList<Model::MyFile *> myFiles;
        QTcpSocket *tcpSocket;
        QList<Model::Connection *> connections;
    };

    inline const QList<Model::MyFile *> &Client::getMyFiles()
    {
        return myFiles;
    }

    inline QList<Model::Connection *> Client::getConnections() const
    {
        return connections;
    }

    inline Client::Status Client::getStatus() const
    {
        return status;
    }
} // namespace Network
