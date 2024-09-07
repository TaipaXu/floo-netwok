#pragma once

#include <tuple>
#include <QObject>

QT_BEGIN_NAMESPACE
class QHttpServer;
QT_END_NAMESPACE

namespace Network
{
    class HttpSender : public QObject
    {
        Q_OBJECT

    public:
        HttpSender(QObject *parent = nullptr);
        ~HttpSender() = default;

        std::tuple<QString, int> startSendFile(const QString &path);

    private:
        void createHttpServer();

    private:
        static QHttpServer *httpServer;
        static int httpServerPort;
        bool visited;
    };
} // namespace Network
