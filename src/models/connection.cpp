#include "./connection.hpp"
#include <QTcpSocket>

namespace Model
{
    Connection::Connection(QTcpSocket *tcpSocket, QObject *parent)
        : QObject(parent), linkType{LinkType::TcpSocket}
    {
        link.tcpSocket = tcpSocket;
    }

    Connection::Connection(const QString &address, QObject *parent)
        : QObject(parent), linkType{LinkType::Address}
    {
        link.address = address;
    }

    Connection::~Connection()
    {
        qDebug() << "connection destructor";
        if (linkType == LinkType::TcpSocket && link.tcpSocket)
        {
            link.tcpSocket->deleteLater();
            link.tcpSocket = nullptr;
        }
    }

    QString Connection::getAddress() const
    {
        if (linkType == LinkType::TcpSocket)
        {
            return link.tcpSocket->peerAddress().toString();
        }
        else
        {
            return link.address;
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
