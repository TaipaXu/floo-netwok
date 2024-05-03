#include "./core.hpp"
#include <QQmlApplicationEngine>
#include "models/channel.hpp"
#include "utils/utils.hpp"

Core::Core()
{
    qmlRegisterType<Model::Channel>("model.channel", 1, 0, "Channel");
    qmlRegisterType<Utils>("utils", 1, 0, "Utils");

    engine = new QQmlApplicationEngine();
    engine->load("qrc:/widgets/MainWindow.qml");
}

Core::~Core()
{
    engine->deleteLater();
}
