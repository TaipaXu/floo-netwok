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
        void fileReceived() const;
        void fileReceiveError() const;
        void fileReceiveDisconnected() const;

    private:
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
} // namespace Network
