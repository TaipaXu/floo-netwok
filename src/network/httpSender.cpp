#include "./httpSender.hpp"
#include <QHttpServer>
#include <QFileInfo>
#include <QUuid>
#include "utils/utils.hpp"

namespace Network
{
    QHttpServer *HttpSender::httpServer = nullptr;
    int HttpSender::httpServerPort = 1024;

    HttpSender::HttpSender(QObject *parent)
        : Sender(parent), visited{false}, size{0}
    {
    }

    Sender::Type HttpSender::getType() const
    {
        return Type::Http;
    }

    std::tuple<QString, int> HttpSender::startSendFile(const QString &path)
    {
        const QFileInfo fileInfo(path);
        if (fileInfo.exists())
        {
            name = fileInfo.fileName();
            size = fileInfo.size();
            emit nameChanged();
            emit sizeChanged();
        }

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

    QString HttpSender::getName() const
    {
        return name;
    }

    QString HttpSender::getSize() const
    {
        if (size != 0)
        {
            return Utils::getReadableSize(size);
        }
        else
        {
            return QStringLiteral();
        }
    }
} // namespace Network
