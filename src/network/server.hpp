#pragma once

#include <QObject>
#include <map>
#include <memory>

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
QT_END_NAMESPACE

namespace Model
{
    class File;
    class MyFile;
} // namespace Model

namespace Network
{
    class Server : public QObject
    {
        Q_OBJECT

    public:
        Server(QObject *parent = nullptr);
        ~Server();

        Q_INVOKABLE bool start(const QString &address, int port);
        Q_INVOKABLE void stop();

    signals:
        void newConnection() const;
        void disconnected() const;

    private slots:
        void handleNewConnection();
        void handleReadyRead();
        void handleDisconnected();

    private:
        QTcpServer *tcpServer;
        std::map<QTcpSocket *, std::list<std::shared_ptr<Model::File>>> socketFiles;
    };
} // namespace Network
