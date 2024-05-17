#include "./core.hpp"
#include <QQmlApplicationEngine>
#include "models/channel.hpp"
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "models/connection.hpp"
#include "network/server.hpp"
#include "network/client.hpp"
#include "utils/utils.hpp"

Core::Core()
{
    qmlRegisterType<Model::Channel>("model.channel", 1, 0, "Channel");
    qmlRegisterType<Model::File>("model.file", 1, 0, "File");
    qmlRegisterType<Model::MyFile>("model.myFile", 1, 0, "MyFile");
    qmlRegisterType<Model::Connection>("model.connection", 1, 0, "Connection");
    qmlRegisterType<Network::Server>("network.server", 1, 0, "Server");
    qmlRegisterType<Network::Client>("network.client", 1, 0, "Client");
    qmlRegisterType<Utils>("utils", 1, 0, "Utils");

    engine = new QQmlApplicationEngine();
    engine->load("qrc:/widgets/MainWindow.qml");
}

Core::~Core()
{
    engine->deleteLater();
}
