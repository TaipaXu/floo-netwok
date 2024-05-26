#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QHttpServer;
QT_END_NAMESPACE

namespace Network
{
    class WebServer : public QObject
    {
        Q_OBJECT

    public:
        WebServer(QObject *parent = nullptr);
        ~WebServer();

        void start(const QString &address);

    private:
        QHttpServer *httpServer;
    };
} // namespace Network
