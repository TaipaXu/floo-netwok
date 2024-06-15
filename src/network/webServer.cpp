#include "./webServer.hpp"
#include <QHttpServer>

namespace Network
{
    WebServer::WebServer(QObject *parent)
        : QObject(parent), httpServer{nullptr}
    {
    }

    WebServer::~WebServer()
    {
        if (httpServer)
        {
            httpServer->deleteLater();
            httpServer = nullptr;
        }
    }

    int WebServer::start(const QString &address)
    {
        httpServer = new QHttpServer(this);
        httpServer->route("/web/<arg>", QHttpServerRequest::Method::Get, [](const QUrl &path, const QHttpServerRequest &request) {
            return QHttpServerResponse::fromFile(QString(":/web/%1").arg(path.toString()));
        });
        int port = 2000;
        while (!httpServer->listen(QHostAddress(address), port))
        {
            port++;
        }
        return port;
    }
} // namespace Network
