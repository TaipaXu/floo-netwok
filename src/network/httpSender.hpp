#pragma once

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
        ~HttpSender();

        int startSendFile(const QString &path);

    private slots:
        void handleNewConnection();

    private:
        QHttpServer *httpServer;
    };
} // namespace Network
