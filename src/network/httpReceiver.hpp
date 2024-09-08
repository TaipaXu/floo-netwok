#pragma once

#include <tuple>
#include <QObject>
#include "./receiver.hpp"

QT_BEGIN_NAMESPACE
class QHttpServer;
QT_END_NAMESPACE

namespace Network
{
    class HttpReceiver : public Receiver
    {
        Q_OBJECT

    public:
        HttpReceiver(QObject *parent = nullptr);
        ~HttpReceiver() = default;

        std::tuple<QString, int> startReceiveFile();

    private:
        Type getType() const override;
        void createHttpServer();
        QString getName() const override;
        QString getSize() const override;

    private:
        static QHttpServer *httpServer;
        static int httpServerPort;
        bool visited;
        QString name;
        long long int size;
    };
} // namespace Network
