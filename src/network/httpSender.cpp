#include "./httpSender.hpp"
#include <QHttpServer>

namespace Network
{
    HttpSender::HttpSender(QObject *parent)
        : QObject(parent), httpServer{nullptr}, visited{false}
    {
    }

    HttpSender::~HttpSender()
    {
    }

    int HttpSender::startSendFile(const QString &path)
    {
        httpServer = new QHttpServer(this);
        connect(httpServer, &QHttpServer::newWebSocketConnection, this, &HttpSender::handleNewConnection);
        httpServer->route("/", QHttpServerRequest::Method::Get, [path, this](const QHttpServerRequest &request) {
            qDebug() << "request" << request.url() << path;
            if (visited)
            {
                return QHttpServerResponse(QHttpServerResponse::StatusCode::NotFound);
            }
            else
            {
                visited = true;
                return QHttpServerResponse::fromFile(path);
            }
        });
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
