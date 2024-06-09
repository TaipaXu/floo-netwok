#include "./httpReceiver.hpp"
#include <memory>
#include <QHttpServer>
#include <QFile>
#include <QSaveFile>
#include <QIODevice>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include "persistence/settings.hpp"

namespace Network
{
    HttpReceiver::HttpReceiver(QObject *parent)
        : QObject(parent), httpServer{nullptr}, visited{false}
    {
    }

    HttpReceiver::~HttpReceiver()
    {
    }

    int HttpReceiver::startReceiveFile()
    {
        httpServer = new QHttpServer(this);
        httpServer->route("/", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
            qDebug() << "startReceiveFile request" << request.url();
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
                    qDebug() << "File saved to" << filePath;
                    QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
                    response.addHeader("Access-Control-Allow-Origin", "*");
                    response.addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
                    return response;
                }

                qDebug() << "Failed to save file.";
                return QHttpServerResponse(QHttpServerResponse::StatusCode::InternalServerError);
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
} // namespace Network
