#include "./tcpSender.hpp"
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include "utils/utils.hpp"

namespace Network
{
    TcpSender::TcpSender(QObject *parent)
        : Sender(parent),
          status{Status::SettingUp},
          tcpServer{nullptr},
          localFile{nullptr},
          totalBytes{0},
          bytesWritten{0},
          bytesToWrite{0},
          payloadSize{64 * 1024},
          outBlock{nullptr}
    {
    }

    Sender::Type TcpSender::getType() const
    {
        return Type::Tcp;
    }

    int TcpSender::startSendFile(const QString &path)
    {
        localFile = new QFile(path, this);
        emit nameChanged();
        emit sizeChanged();

        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &TcpSender::handleNewConnection);
        int port = 1024;
        while (!tcpServer->listen(QHostAddress::Any, port))
        {
            port++;
        }
        return port;
    }

    QString TcpSender::getStatusName() const
    {
        switch (status)
        {
        case Status::SettingUp:
            return tr("Setting up");
        case Status::Sending:
            return tr("Sending");
        case Status::Finished:
            return tr("Finished");
        case Status::Canceled:
            return tr("Canceled");
        case Status::Error:
            return tr("Error");
        }
    }

    QString TcpSender::getName() const
    {
        if (localFile)
        {
            const QFileInfo info(*localFile);
            return info.fileName();
        }
        else
        {
            return QStringLiteral();
        }
    }

    QString TcpSender::getSize() const
    {
        if (localFile)
        {
            return Utils::getReadableSize(localFile->size());
        }
        else
        {
            return QStringLiteral();
        }
    }

    double TcpSender::getProgress() const
    {
        return totalBytes == 0 ? 0 : static_cast<double>(totalBytes - bytesToWrite) / totalBytes;
    }

    void TcpSender::deleteTcpServer()
    {
        if (tcpServer)
        {
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }

        if (localFile)
        {
            localFile->deleteLater();
            localFile = nullptr;
        }
    }

    void TcpSender::handleNewConnection()
    {
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::bytesWritten, this, &TcpSender::sendFileData);
        connect(socket, &QTcpSocket::errorOccurred, this, &TcpSender::handleError);
        connect(socket, &QTcpSocket::disconnected, this, &TcpSender::handleDisconnected);
        localFile->open(QFile::ReadOnly);
        totalBytes = localFile->size();
        QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
        sendOut.setVersion(QDataStream::Qt_6_7);
        QFileInfo fileInfo(*localFile);
        const QString currentFileName = fileInfo.fileName();
        sendOut << static_cast<long long>(0) << static_cast<long long>(0) << currentFileName.toUtf8().constData();
        totalBytes += outBlock.size();
        sendOut.device()->seek(0);
        sendOut << totalBytes << static_cast<long long>(outBlock.size() - sizeof(long long) * 2);
        bytesToWrite = totalBytes - socket->write(outBlock);
        outBlock.resize(0);
    }

    void TcpSender::sendFileData()
    {
        QTcpSocket *const socket = qobject_cast<QTcpSocket *>(sender());
        if (bytesToWrite > 0)
        {
            if (status != Status::Sending)
            {
                status = Status::Sending;
                emit statusChanged();
            }
            outBlock = localFile->read(std::min(bytesToWrite, payloadSize));
            bytesWritten = socket->write(outBlock);
            bytesToWrite -= bytesWritten;
            outBlock.resize(0);
            emit progressChanged();
        }
        else
        {
            localFile->close();
            deleteTcpServer();

            status = Status::Finished;
            emit statusChanged();
            emit fileSendFinished();
        }
    }

    void TcpSender::handleError(QAbstractSocket::SocketError socketError)
    {
        deleteTcpServer();
        status = Status::Error;
        emit fileSendError();
        emit statusChanged();
    }

    void TcpSender::handleDisconnected()
    {
        deleteTcpServer();
        emit fileSendDisconnected();
    }
} // namespace Network
