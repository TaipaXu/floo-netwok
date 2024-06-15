#include "./httpSender.hpp"
#include <QHttpServer>
#include <QFileInfo>

namespace Network
{
    HttpSender::HttpSender(QObject *parent)
        : QObject(parent), httpServer{nullptr}, visited{false}
    {
    }

    int HttpSender::startSendFile(const QString &path)
    {
        httpServer = new QHttpServer(this);
        httpServer->route("/", QHttpServerRequest::Method::Get, [path, this](const QHttpServerRequest &request) {
            if (visited)
            {
                return QHttpServerResponse(QHttpServerResponse::StatusCode::NotFound);
            }
            else
            {
                visited = true;
                QHttpServerResponse response{QHttpServerResponse::fromFile(path)};
                response.addHeader("Content-Disposition", QStringLiteral("attachment; filename=%1").arg(QFileInfo(path).fileName()).toUtf8());
                return response;
            }
        });
        int port = 1024;
        while (!httpServer->listen(QHostAddress::Any, port))
        {
            port++;
        }
        return port;
    }
} // namespace Network
