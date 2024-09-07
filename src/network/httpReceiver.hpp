#pragma once

#include <tuple>
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
        ~HttpReceiver() = default;

        std::tuple<QString, int> startReceiveFile();

    private:
        void createHttpServer();

    private:
        static QHttpServer *httpServer;
        static int httpServerPort;
        bool visited;
    };
} // namespace Network
