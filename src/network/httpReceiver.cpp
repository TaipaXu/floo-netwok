#include "./httpReceiver.hpp"
#include <QHttpServer>
#include <QSaveFile>
#include <QIODevice>
#include <QUuid>
#include "persistence/settings.hpp"

namespace Network
{
    QHttpServer *HttpReceiver::httpServer = nullptr;
    int HttpReceiver::httpServerPort = 1024;

    HttpReceiver::HttpReceiver(QObject *parent)
        : QObject(parent), visited{false}
    {
    }

    std::tuple<QString, int> HttpReceiver::startReceiveFile()
    {
        createHttpServer();

        const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);

        httpServer->route("/" + id, QHttpServerRequest::Method::Options, [this](const QHttpServerRequest &request) {
            QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
            response.addHeader("Access-Control-Allow-Origin", "*");
            response.addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            response.addHeader("Access-Control-Allow-Headers", "*");
            return response;
        });

        httpServer->route("/" + id, QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
            if (visited)
            {
                QHttpServerResponse response(QHttpServerResponse::StatusCode::NotFound);
                response.addHeader("Access-Control-Allow-Origin", "*");
                response.addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
                return response;
            }
            else
            {
                visited = true;

                QString fileName;
                for (auto &&header : request.headers())
                {
                    const QString key = header.first;
                    if (key.toLower() == QStringLiteral("filename"))
                    {
                        fileName = header.second;
                    }
                }
                std::unique_ptr<Persistence::Settings> settings = std::make_unique<Persistence::Settings>();
                QString filePath = settings->getDownloadPath() + "/" + fileName;
                QSaveFile file(filePath);
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(request.body());
                    file.commit();
                    QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
                    response.addHeader("Access-Control-Allow-Origin", "*");
                    response.addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
                    return response;
                }

                return QHttpServerResponse(QHttpServerResponse::StatusCode::InternalServerError);
            }
        });

        return {id, httpServerPort};
    }

    void HttpReceiver::createHttpServer()
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
