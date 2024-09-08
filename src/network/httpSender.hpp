#pragma once

#include <tuple>
#include <QObject>
#include "./sender.hpp"

QT_BEGIN_NAMESPACE
class QHttpServer;
QT_END_NAMESPACE

namespace Network
{
    class HttpSender : public Sender
    {
        Q_OBJECT

    public:
        HttpSender(QObject *parent = nullptr);
        ~HttpSender() = default;

        std::tuple<QString, int> startSendFile(const QString &path);

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
