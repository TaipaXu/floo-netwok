#pragma once

#include <QObject>
#include <QAbstractSocket>
#include <map>
#include <memory>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

namespace Model
{
    class File;
    class MyFile;
} // namespace Model

namespace Network
{
    class Client : public QObject
    {
        Q_OBJECT

    public:
        Client(QObject *parent = nullptr);
        ~Client();

        Q_INVOKABLE void start(const QString &address, int port);
        Q_INVOKABLE void stop();

    signals:
        void connected() const;
        void disconnected() const;
        void connectError() const;

    private slots:
        void handleConnected() const;
        void handleReadyRead();
        void handleError(QAbstractSocket::SocketError socketError) const;
        void handleDisconnected();

    private:
        QTcpSocket *tcpSocket;
    };
} // namespace Network
