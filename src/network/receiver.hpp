#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QFile;
QT_END_NAMESPACE

namespace Network
{
    class Receiver : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
        Q_PROPERTY(QString statusName READ getStatusName NOTIFY statusChanged)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        Q_PROPERTY(QString size READ getSize NOTIFY sizeChanged)

    public:
        enum class Status
        {
            SettingUp,
            Receiving,
            Finished,
            Canceled,
            Error
        };

    public:
        Receiver(QObject *parent = nullptr);
        ~Receiver();

        void startReceiveFile(const QString &ip, int port);

    signals:
        void progressChanged(long long bytesReceived, long long totalBytes) const;
        void statusChanged(Status status) const;
        void nameChanged() const;
        void sizeChanged() const;
        void fileReceived() const;
        void fileReceiveError() const;
        void fileReceiveDisconnected() const;

    private:
        Status getStatus() const;
        QString getStatusName() const;
        QString getName() const;
        QString getSize() const;
        void deleteTcpSocket();

    private slots:
        void readMessage();
        void handleError();
        void handleDisconnected();

    private:
        Status status;
        QTcpSocket *tcpSocket;
        QFile *localFile;
        long long totalBytes;
        long long bytesReceived;
        long long fileNameSize;
        QByteArray inBlock;
    };

    inline Receiver::Status Receiver::getStatus() const
    {
        return status;
    }
} // namespace Network
