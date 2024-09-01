#include "./httpReceiver.hpp"
#include <QHttpServer>
#include <QSaveFile>
#include <QIODevice>
#include "persistence/settings.hpp"

namespace Network
{
    HttpReceiver::HttpReceiver(QObject *parent)
        : QObject(parent), httpServer{nullptr}, visited{false}
    {
    }

    int HttpReceiver::startReceiveFile()
    {
        httpServer = new QHttpServer(this);

        httpServer->route("/", QHttpServerRequest::Method::Options, [this](const QHttpServerRequest &request) {
            QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
            response.addHeader("Access-Control-Allow-Origin", "*");
            response.addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            response.addHeader("Access-Control-Allow-Headers", "*");
            return response;
        });

        httpServer->route("/", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
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
                    if (key == QStringLiteral("Filename"))
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
        int port = 1024;
        while (!httpServer->listen(QHostAddress::Any, port))
        {
            port++;
        }
        return port;
    }
} // namespace Network
