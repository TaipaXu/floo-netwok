#include "./httpReceiver.hpp"
#include <QHttpServer>
#include <QFile>
#include <QSaveFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
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

                const QByteArray requestData = request.body();
                const QJsonObject json = QJsonDocument::fromJson(requestData).object();
                std::unique_ptr<Persistence::Settings> settings = std::make_unique<Persistence::Settings>();
                QString filePath = settings->getDownloadPath() + "/" + json["name"].toString();
                QSaveFile file(filePath);
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(QByteArray::fromBase64(json["file"].toString().toUtf8()));
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
