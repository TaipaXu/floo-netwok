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
QT_END_NAMESPACE

namespace Model
{
    class Connection : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString address READ getAddress CONSTANT)
        Q_PROPERTY(QList<Model::File *> files READ getFiles NOTIFY filesChanged)

        enum class LinkType
        {
            TcpSocket,
            Address
        };

    public:
        Connection(QTcpSocket *tcpSocket, QObject *parent = nullptr);
        Connection(const QString &address, QObject *parent = nullptr);
        ~Connection();

        QTcpSocket *const getTcpSocket() const;
        QString getAddress() const;
        QList<Model::File *> getFiles() const;
        void setFiles(const QList<Model::File *> &files);
        void removeFile(Model::File *file);

    signals:
        void filesChanged() const;

    private:
        LinkType linkType;
        QTcpSocket *tcpSocket = nullptr;
        QString address;
        QList<Model::File *> files;
    };

    inline QTcpSocket *const Connection::getTcpSocket() const
    {
        return tcpSocket;
    }

    inline QList<Model::File *> Connection::getFiles() const
    {
        return files;
    }
} // namespace Model
