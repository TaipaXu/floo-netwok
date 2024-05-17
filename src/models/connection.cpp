#include "./connection.hpp"
#include <QTcpSocket>

namespace Model
{
    Connection::Connection(QTcpSocket *tcpSocket, QObject *parent)
        : QObject(parent), tcpSocket{tcpSocket}
    {
    }

    Connection::~Connection()
    {
        qDebug() << "connection destructor";
        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }
    }

    QString Connection::getAddress() const
    {
        return tcpSocket->peerAddress().toString();
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
