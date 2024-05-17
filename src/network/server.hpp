#pragma once

#include <QObject>
#include <QList>
#include "models/connection.hpp"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
QT_END_NAMESPACE

namespace Model
{
    class File;
    class MyFile;
} // namespace Model

namespace Network
{
    class Server : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)

    public:
        Server(QObject *parent = nullptr);
        ~Server();

        Q_INVOKABLE bool start(const QString &address, int port);
        Q_INVOKABLE void stop();
        QList<Model::Connection *> getConnections() const;

    signals:
        void newConnection() const;
        void disconnected() const;
        void connectionsChanged() const;

    private slots:
        void handleNewConnection();
        void handleReadyRead();
        void handleDisconnected();

    private:
        void addClientFiles(QTcpSocket *socket, const QJsonArray &filesArray);

    private:
        QTcpServer *tcpServer;
        QList<Model::Connection *> connections;
    };

    inline QList<Model::Connection *> Server::getConnections() const
    {
        return connections;
    }
} // namespace Network
