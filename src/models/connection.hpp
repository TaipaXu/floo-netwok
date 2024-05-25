#pragma once

#include <QObject>
#include <QList>

namespace Model
{
    class File;
    class MyFile;
} // namespace Model

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QWebSocket;
QT_END_NAMESPACE

namespace Model
{
    class Connection : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString address READ getAddress CONSTANT)
        Q_PROPERTY(QList<Model::File *> files READ getFiles NOTIFY filesChanged)

    public:
        enum class LinkType
        {
            TcpSocket,
            WSSocket,
            Address
        };

    public:
        Connection(QTcpSocket *tcpSocket, QObject *parent = nullptr);
        Connection(QWebSocket *wsSocket, QObject *parent = nullptr);
        Connection(const QString &address, QObject *parent = nullptr);
        ~Connection();

        LinkType getLinkType() const;
        QTcpSocket *const getTcpSocket() const;
        QWebSocket *const getWsSocket() const;
        QString getAddress() const;
        QList<Model::File *> getFiles() const;
        void setFiles(const QList<Model::File *> &files);
        void removeFile(Model::File *file);

    signals:
        void filesChanged() const;

    private:
        LinkType linkType;
        QTcpSocket *tcpSocket = nullptr;
        QWebSocket *wsSocket = nullptr;
        QString address;
        QList<Model::File *> files;
    };

    inline Connection::LinkType Connection::getLinkType() const
    {
        return linkType;
    }

    inline QTcpSocket *const Connection::getTcpSocket() const
    {
        return tcpSocket;
    }

    inline QWebSocket *const Connection::getWsSocket() const
    {
        return wsSocket;
    }

    inline QList<Model::File *> Connection::getFiles() const
    {
        return files;
    }
} // namespace Model
