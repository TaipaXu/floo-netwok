#pragma once

#include <map>
#include <QObject>
#include <QAbstractSocket>
#include "models/connection.hpp"

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

namespace Model
{
    class File;
    class MyFile;
} // namespace Model

namespace Network
{
    class Client : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
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
        Client(QObject *parent = nullptr);
        ~Client();

        Q_INVOKABLE void start(const QString &address, int port);
        Q_INVOKABLE void stop();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *const myFile);
        Q_INVOKABLE void requestDownloadFile(const Model::File *const file);

    signals:
        void statusChanged() const;
        void connectError() const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        Status getStatus() const;
        QList<Model::Connection *> getConnections() const;
        const QList<Model::MyFile *> &getMyFiles() const;
        void sendFilesInfoToServer() const;
        void handleRequestFiles(const QJsonObject &ipFiles);
        void handleRequestPrepareUploadFile(const QString &recordId, const QString &fileId) const;
        void handleRequestPrepareUploadFileForWeb(const QString &recordId, const QString &fileId) const;
        void handleRequestUploadFileReady(const QString &ip, int port, const QString &fileId);
        void handleRequestPrepareDownloadFileFromWeb(const QString &recordId) const;

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
        std::map<QString, int> requestingFiles;
    };

    inline Client::Status Client::getStatus() const
    {
        return status;
    }

    inline QList<Model::Connection *> Client::getConnections() const
    {
        return connections;
    }

    inline const QList<Model::MyFile *> &Client::getMyFiles() const
    {
        return myFiles;
    }
} // namespace Network
