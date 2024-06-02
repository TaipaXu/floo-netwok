#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QHttpServer;
QT_END_NAMESPACE

namespace Network
{
    class HttpReceiver : public QObject
    {
        Q_OBJECT

    public:
        HttpReceiver(QObject *parent = nullptr);
        ~HttpReceiver();

        int startReceiveFile();

    private:
        QHttpServer *httpServer;
        bool visited;
    };
} // namespace Network
