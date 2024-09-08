#pragma once

#include <QObject>
#include <QAbstractSocket>
#include "./receiver.hpp"

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QFile;
QT_END_NAMESPACE

namespace Network
{
    class TcpReceiver : public Receiver
    {
        Q_OBJECT

        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
        Q_PROPERTY(QString statusName READ getStatusName NOTIFY statusChanged)
        Q_PROPERTY(double progress READ getProgress NOTIFY progressChanged)

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
        TcpReceiver(QObject *parent = nullptr);
        ~TcpReceiver() = default;

        void startReceiveFile(const QString &ip, int port);

    signals:
        void statusChanged() const;
        void progressChanged() const;
        void fileReceived() const;
        void fileReceiveError() const;
        void fileReceiveDisconnected() const;

    private:
        Type getType() const override;
        Status getStatus() const;
        QString getStatusName() const;
        QString getName() const override;
        QString getSize() const override;
        double getProgress() const;
        void deleteTcpSocket();

    private slots:
        void readMessage();
        void handleError(QAbstractSocket::SocketError socketError);
        void handleDisconnected();

    private:
        const QString id;
        Status status;
        QTcpSocket *tcpSocket;
        QFile *localFile;
        long long totalBytes;
        long long bytesReceived;
        long long fileNameSize;
        QByteArray inBlock;
    };

    inline TcpReceiver::Status TcpReceiver::getStatus() const
    {
        return status;
    }
} // namespace Network
