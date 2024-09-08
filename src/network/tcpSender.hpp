#pragma once

#include <QObject>
#include <QAbstractSocket>
#include "./sender.hpp"
#include "models/file.hpp"
#include "models/myFile.hpp"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QFile;
QT_END_NAMESPACE

namespace Network
{
    class TcpSender : public Sender
    {
        Q_OBJECT

        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
        Q_PROPERTY(QString statusName READ getStatusName NOTIFY statusChanged)
        Q_PROPERTY(double progress READ getProgress NOTIFY progressChanged)

    public:
        enum class Status
        {
            SettingUp,
            Sending,
            Finished,
            Canceled,
            Error
        };
        Q_ENUM(Status)

    public:
        TcpSender(QObject *parent = nullptr);
        ~TcpSender() = default;

        int startSendFile(const QString &path);

    signals:
        void statusChanged() const;
        void progressChanged() const;
        void fileSendFinished() const;
        void fileSendError() const;
        void fileSendDisconnected() const;

    private:
        Type getType() const override;
        Status getStatus() const;
        QString getStatusName() const;
        QString getName() const override;
        QString getSize() const override;
        double getProgress() const;
        void deleteTcpServer();

    private slots:
        void handleNewConnection();
        void sendFileData();
        void handleError(QAbstractSocket::SocketError socketError);
        void handleDisconnected();

    private:
        const QString id;
        Status status;
        QTcpServer *tcpServer;
        QFile *localFile;
        long long totalBytes;
        long long bytesWritten;
        long long bytesToWrite;
        long long payloadSize;
        QByteArray outBlock;
    };

    inline TcpSender::Status TcpSender::getStatus() const
    {
        return status;
    }
} // namespace Network
