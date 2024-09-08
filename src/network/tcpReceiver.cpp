#include "tcpReceiver.hpp"
#include <memory>
#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include "persistence/settings.hpp"
#include "utils/utils.hpp"

namespace Network
{
    TcpReceiver::TcpReceiver(QObject *parent)
        : Receiver(parent),
          status{Status::SettingUp},
          tcpSocket{nullptr},
          localFile{nullptr},
          totalBytes{0},
          bytesReceived{0},
          fileNameSize{0},
          inBlock{}
    {
    }

    Receiver::Type TcpReceiver::getType() const
    {
        return Type::Tcp;
    }

    void TcpReceiver::startReceiveFile(const QString &ip, int port)
    {
        tcpSocket = new QTcpSocket(this);
        connect(tcpSocket, &QTcpSocket::readyRead, this, &TcpReceiver::readMessage);
        connect(tcpSocket, &QTcpSocket::disconnected, this, &TcpReceiver::handleDisconnected);
        connect(tcpSocket, &QTcpSocket::errorOccurred, this, &TcpReceiver::handleError);
        tcpSocket->connectToHost(ip, port);
    }

    QString TcpReceiver::getStatusName() const
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

    QString TcpReceiver::getName() const
    {
        if (localFile)
        {
            return QFileInfo(*localFile).fileName();
        }
        else
        {
            return QStringLiteral();
        }
    }

    QString TcpReceiver::getSize() const
    {
        return Utils::getReadableSize(totalBytes);
    }

    double TcpReceiver::getProgress() const
    {
        return totalBytes == 0 ? 0 : static_cast<double>(bytesReceived) / totalBytes;
    }

    void TcpReceiver::deleteTcpSocket()
    {
        if (tcpSocket)
        {
            tcpSocket->deleteLater();
            tcpSocket = nullptr;
        }

        if (localFile)
        {
            localFile->deleteLater();
            localFile = nullptr;
        }
    }

    void TcpReceiver::readMessage()
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
                std::unique_ptr<Persistence::Settings> settings = std::make_unique<Persistence::Settings>();
                QString filePath = settings->getDownloadPath() + "/" + QString(name);
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

    void TcpReceiver::handleError(QAbstractSocket::SocketError socketError)
    {
        deleteTcpSocket();
        status = Status::Error;
        emit fileReceiveError();
        emit statusChanged();
    }

    void TcpReceiver::handleDisconnected()
    {
        deleteTcpSocket();
        emit fileReceiveDisconnected();
    }
} // namespace Network
