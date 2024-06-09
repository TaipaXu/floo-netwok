#include "./core.hpp"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "models/channel.hpp"
#include "models/file.hpp"
#include "models/myFile.hpp"
#include "models/connection.hpp"
#include "network/server.hpp"
#include "network/client.hpp"
#include "network/sendManager.hpp"
#include "network/receiveManager.hpp"
#include "network/sender.hpp"
#include "network/receiver.hpp"
#include "persistence/settings.hpp"
#include "utils/utils.hpp"

Core::Core()
{
    qmlRegisterType<Model::Channel>("model.channel", 1, 0, "Channel");
    qmlRegisterType<Model::File>("model.file", 1, 0, "File");
    qmlRegisterType<Model::MyFile>("model.myFile", 1, 0, "MyFile");
    qmlRegisterType<Model::Connection>("model.connection", 1, 0, "Connection");
    qmlRegisterType<Network::Server>("network.server", 1, 0, "Server");
    qmlRegisterType<Network::Client>("network.client", 1, 0, "Client");
    QJSEngine::setObjectOwnership(Network::SendManager::getInstance(), QQmlEngine::CppOwnership);
    QJSEngine::setObjectOwnership(Network::ReceiveManager::getInstance(), QQmlEngine::CppOwnership);
    qmlRegisterType<Network::Sender>("network.sender", 1, 0, "Sender");
    qmlRegisterType<Network::Receiver>("network.receiver", 1, 0, "Receiver");
    qmlRegisterType<Persistence::Settings>("persistence.settings", 1, 0, "Settings");
    qmlRegisterType<Utils>("utils", 1, 0, "Utils");

    engine = new QQmlApplicationEngine();
    engine->rootContext()->setContextProperty("SendManager", Network::SendManager::getInstance());
    engine->rootContext()->setContextProperty("ReceiveManager", Network::ReceiveManager::getInstance());
    engine->load("qrc:/widgets/MainWindow.qml");
}

Core::~Core()
{
    engine->deleteLater();
}
