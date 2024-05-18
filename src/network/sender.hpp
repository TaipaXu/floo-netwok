#pragma once

#include <QObject>
#include <QAbstractSocket>
#include "models/file.hpp"
#include "models/myFile.hpp"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QFile;
QT_END_NAMESPACE

namespace Network
{
    class Sender : public QObject
    {
        Q_OBJECT

    public:
        enum class Status
        {
            SettingUp,
            Sending,
            Finished,
            Canceled,
            Error
        };

    public:
        Sender(QObject *parent = nullptr);
        ~Sender();

        int startSendFile(const QString &path);

    signals:
        void progressChanged(long long bytesReceived, long long totalBytes) const;
        void statusChanged(Status status) const;
        void fileSendFinished() const;
        void fileSendError() const;
        void fileSendDisconnected() const;

    private:
        void deleteTcpServer();

    private slots:
        void handleNewConnection();
        void sendFileData();
        void handleError(QAbstractSocket::SocketError socketError);
        void handleDisconnected();

    private:
        Status status;
        QFile *localFile;
        QTcpServer *tcpServer;
        long long totalBytes;
        long long bytesWritten;
        long long bytesToWrite;
        long long payloadSize;
        QByteArray outBlock;
    };
} // namespace Network
