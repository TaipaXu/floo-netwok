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

        Q_PROPERTY(QList<Model::MyFile *> myFiles READ getMyFiles NOTIFY myFilesChanged)
        Q_PROPERTY(QList<Model::Connection *> connections READ getConnections NOTIFY connectionsChanged)

    public:
        Client(QObject *parent = nullptr);
        ~Client();

        Q_INVOKABLE void start(const QString &address, int port);
        Q_INVOKABLE void stop();
        const QList<Model::MyFile *> &getMyFiles();
        Q_INVOKABLE void addMyFiles(const QList<QUrl> &myFiles);
        Q_INVOKABLE void removeMyFile(Model::MyFile *myFile);
        QList<Model::Connection *> getConnections() const;

    signals:
        void connected() const;
        void disconnected() const;
        void connectError() const;
        void connectionsChanged() const;
        void myFilesChanged() const;

    private:
        void handleRequestFiles(const QJsonObject &ipFiles);
        void sendFilesInfoToServer() const;

    private slots:
        void handleConnected() const;
        void handleReadyRead();
        void handleError(QAbstractSocket::SocketError socketError) const;
        void handleDisconnected();

    private:
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
} // namespace Network
