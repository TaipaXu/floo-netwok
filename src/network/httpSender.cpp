#include "./httpSender.hpp"
#include <QHttpServer>

namespace Network
{
    HttpSender::HttpSender(QObject *parent)
        : QObject(parent), httpServer{nullptr}
    {
    }

    HttpSender::~HttpSender()
    {
    }

    int HttpSender::startSendFile(const QString &path)
    {
        httpServer = new QHttpServer(this);
        httpServer->route("/", QHttpServerRequest::Method::Get, [path](const QHttpServerRequest &request) {
            qDebug() << "request" << request.url() << path;
            return QHttpServerResponse::fromFile(path);
        });
        connect(httpServer, &QHttpServer::newWebSocketConnection, this, &HttpSender::handleNewConnection);
        int port = 1000;
        while (!httpServer->listen(QHostAddress::Any, port))
        {
            port++;
        }
        qDebug() << "port" << port;
        return port;
    }

    void HttpSender::handleNewConnection()
    {
        qDebug() << "new connection";
    }
} // namespace Network
