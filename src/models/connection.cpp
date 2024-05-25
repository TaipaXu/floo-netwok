#include "./connection.hpp"
#include <QTcpSocket>
#include <QWebSocket>
#include "models/file.hpp"

namespace Model
{
    Connection::Connection(QTcpSocket *tcpSocket, QObject *parent)
        : QObject(parent), linkType{LinkType::TcpSocket}, tcpSocket{tcpSocket}
    {
    }

    Connection::Connection(QWebSocket *wsSocket, QObject *parent)
        : QObject(parent), linkType{LinkType::WSSocket}, wsSocket{wsSocket}
    {
    }

    Connection::Connection(const QString &address, QObject *parent)
        : QObject(parent), linkType{LinkType::Address}, tcpSocket{nullptr}, address{address}
    {
    }

    Connection::~Connection()
    {
        qDebug() << "connection destructor";
        for (auto &&file : files)
        {
            file->deleteLater();
        }
    }

    QString Connection::getAddress() const
    {
        qDebug() << "getAddress";
        if (linkType == LinkType::TcpSocket)
        {
            return tcpSocket->peerAddress().toString();
        }
        else if (linkType == LinkType::WSSocket)
        {
            return wsSocket->peerAddress().toString();
        }
        else
        {
            return address;
        }
    }

    void Connection::setFiles(const QList<Model::File *> &files)
    {
        qDeleteAll(this->files);
        this->files = files;
        emit filesChanged();
    }

    void Connection::removeFile(Model::File *file)
    {
        files.removeOne(file);
        emit filesChanged();
    }
} // namespace Model
