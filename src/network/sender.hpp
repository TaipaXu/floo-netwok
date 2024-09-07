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

        Q_PROPERTY(QString id MEMBER id CONSTANT)
        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
        Q_PROPERTY(QString statusName READ getStatusName NOTIFY statusChanged)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        Q_PROPERTY(QString size READ getSize NOTIFY sizeChanged)
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
        Sender(QObject *parent = nullptr);
        ~Sender() = default;

        int startSendFile(const QString &path);

    signals:
        void statusChanged() const;
        void nameChanged() const;
        void sizeChanged() const;
        void progressChanged() const;
        void fileSendFinished() const;
        void fileSendError() const;
        void fileSendDisconnected() const;

    private:
        Status getStatus() const;
        QString getStatusName() const;
        QString getName() const;
        QString getSize() const;
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

    inline Sender::Status Sender::getStatus() const
    {
        return status;
    }
} // namespace Network
