#pragma once

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

        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)

    public:
        Client(QObject *parent = nullptr);
        ~Client();

        Q_INVOKABLE void start(const QString &address, int port);
        Q_INVOKABLE void stop();
        QList<Model::Connection *> getConnections() const;

    signals:
        void connected() const;
        void disconnected() const;
        void connectError() const;
        void connectionsChanged() const;

    private:
        void handleRequestFiles(const QJsonObject &ipFiles);

    private slots:
        void handleConnected() const;
        void handleReadyRead();
        void handleError(QAbstractSocket::SocketError socketError) const;
        void handleDisconnected();

    private:
        QTcpSocket *tcpSocket;
        QList<Model::Connection *> connections;
    };

    inline QList<Model::Connection *> Client::getConnections() const
    {
        return connections;
    }
} // namespace Network
