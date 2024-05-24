#include "receiver.hpp"
#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include "utils/utils.hpp"

namespace Network
{
    Receiver::Receiver(QObject *parent)
        : QObject(parent), status{Status::SettingUp}, tcpSocket{nullptr}, localFile{nullptr}, totalBytes{0}, bytesReceived{0}, fileNameSize{0}, inBlock{}
    {
    }

    Receiver::~Receiver()
    {
    }

    void Receiver::startReceiveFile(const QString &ip, int port)
    {
        tcpSocket = new QTcpSocket(this);
        connect(tcpSocket, &QTcpSocket::readyRead, this, &Receiver::readMessage);
        connect(tcpSocket, &QTcpSocket::disconnected, this, &Receiver::handleDisconnected);
        connect(tcpSocket, &QTcpSocket::errorOccurred, this, &Receiver::handleError);
        tcpSocket->connectToHost(ip, port);
    }

    QString Receiver::getStatusName() const
    {
        switch (status)
        {
        case Status::SettingUp:
            return tr("Setting up");
        case Status::Receiving:
            return tr("Receiving");
        case Status::Finished:
            return tr("Finished");
        case Status::Canceled:
            return tr("Canceled");
        case Status::Error:
            return tr("Error");
        }
    }

    QString Receiver::getName() const
    {
        if (localFile)
        {
            return QFileInfo(*localFile).fileName();
        }
        return QString();
    }

    QString Receiver::getSize() const
    {
        return Utils::getReadableSize(totalBytes);
    }

    double Receiver::getProgress() const
    {
        return totalBytes == 0 ? 0 : static_cast<double>(bytesReceived) / totalBytes;
    }

    void Receiver::deleteTcpSocket()
    {
        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }
    }

    void Receiver::readMessage()
    {
        QDataStream in(tcpSocket);
        in.setVersion(QDataStream::Qt_6_7);
        if (bytesReceived <= sizeof(long long) * 2)
        {
            if ((tcpSocket->bytesAvailable() >= sizeof(long long) * 2) && (fileNameSize == 0))
            {
                in >> totalBytes >> fileNameSize;
                bytesReceived += sizeof(long long) * 2;
                emit sizeChanged();
            }
            if ((tcpSocket->bytesAvailable() >= fileNameSize) && (fileNameSize != 0))
            {
                char *name;
                in >> name;
                bytesReceived += fileNameSize;
                QString filePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + QString(name);
                filePath = QDir::toNativeSeparators(filePath);
                localFile = new QFile(filePath, this);
                status = Status::Receiving;
                emit statusChanged();
                emit nameChanged();
                if (!localFile->open(QFile::WriteOnly))
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        if (bytesReceived < totalBytes)
        {
            bytesReceived += tcpSocket->bytesAvailable();
            inBlock = tcpSocket->readAll();
            localFile->write(inBlock);
            inBlock.resize(0);

            emit progressChanged();
        }
        if (bytesReceived == totalBytes)
        {
            localFile->close();
            deleteTcpSocket();

            status = Status::Finished;
            emit fileReceived();
            emit statusChanged();
        }
    }

    void Receiver::handleError()
    {
        deleteTcpSocket();
        status = Status::Error;
        emit fileReceiveError();
        emit statusChanged();
    }

    void Receiver::handleDisconnected()
    {
        deleteTcpSocket();
        emit fileReceiveDisconnected();
    }
} // namespace Network
