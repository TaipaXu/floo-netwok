#include "./sender.hpp"
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>

namespace Network
{
    Sender::Sender(QObject *parent)
        : QObject(parent), status{Status::SettingUp}, tcpServer{nullptr}, totalBytes{0}, bytesWritten{0}, bytesToWrite{0}, payloadSize{64 * 1024}, outBlock{nullptr}
    {
    }

    Sender::~Sender()
    {
    }

    int Sender::startSendFile(const QString &path)
    {
        localFile = new QFile(path, this);
        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &Sender::handleNewConnection);
        int port = 1000;
        while (!tcpServer->listen(QHostAddress::Any, port))
        {
            port++;
        }
        return port;
    }

    void Sender::deleteTcpServer()
    {
        if (tcpServer)
        {
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }
    }

    void Sender::handleNewConnection()
    {
        QTcpSocket *const socket = tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::bytesWritten, this, &Sender::sendFileData);
        connect(socket, &QTcpSocket::errorOccurred, this, &Sender::handleError);
        connect(socket, &QTcpSocket::disconnected, this, &Sender::handleDisconnected);
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

    void Sender::sendFileData()
    {
        QTcpSocket *const socket = qobject_cast<QTcpSocket *>(sender());
        if (bytesToWrite > 0)
        {
            outBlock = localFile->read(std::min(bytesToWrite, payloadSize));
            bytesWritten = socket->write(outBlock);
            bytesToWrite -= bytesWritten;
            outBlock.resize(0);
            emit progressChanged(totalBytes - bytesToWrite, totalBytes);
        }
        else
        {
            localFile->close();
            status = Status::Finished;
            emit statusChanged(Status::Finished);
            emit fileSendFinished();

            deleteTcpServer();
        }
    }

    void Sender::handleError(QAbstractSocket::SocketError socketError)
    {
        deleteTcpServer();
        status = Status::Error;
        emit fileSendError();
        emit statusChanged(Status::Error);
    }

    void Sender::handleDisconnected()
    {
        deleteTcpServer();
        emit fileSendDisconnected();
    }
} // namespace Network
