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

        Q_PROPERTY(QList<Model::File *> myFiles READ getMyFiles NOTIFY myFilesChanged)
        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)

    public:
        Server(QObject *parent = nullptr);
        ~Server();

        Q_INVOKABLE bool start(const QString &address, int port);
        Q_INVOKABLE void stop();
        const QList<Model::File *> &getMyFiles();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *myFile);
        QList<Model::Connection *> getConnections() const;

    signals:
        void newConnection() const;
        void disconnected() const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        void addClientFiles(QTcpSocket *socket, const QJsonArray &filesArray);
        void broadcastFiles() const;

    private slots:
        void handleNewConnection();
        void handleReadyRead();
        void handleDisconnected();

    private:
        QList<Model::File *> myFiles;
        QTcpServer *tcpServer;
        QList<Model::Connection *> connections;
    };

    inline const QList<Model::File *> &Server::getMyFiles()
    {
        return myFiles;
    }

    inline QList<Model::Connection *> Server::getConnections() const
    {
        return connections;
    }
} // namespace Network
