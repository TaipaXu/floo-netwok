#include "./httpSender.hpp"
#include <QHttpServer>
#include <QFileInfo>
#include <QUuid>

namespace Network
{
    QHttpServer *HttpSender::httpServer = nullptr;
    int HttpSender::httpServerPort = 1024;

    HttpSender::HttpSender(QObject *parent)
        : QObject(parent), visited{false}
    {
    }

    std::tuple<QString, int> HttpSender::startSendFile(const QString &path)
    {
        createHttpServer();

        const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        httpServer->route("/" + id, QHttpServerRequest::Method::Get, [path, this](const QHttpServerRequest &request) {
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

        return {id, httpServerPort};
    }

    void HttpSender::createHttpServer()
    {
        if (!httpServer)
        {
            httpServer = new QHttpServer();

            while (!httpServer->listen(QHostAddress::Any, httpServerPort))
            {
                httpServerPort++;
            }
        }
    }
} // namespace Network
