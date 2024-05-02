#include "./core.hpp"
#include <QQmlApplicationEngine>
#include "utils/utils.hpp"

Core::Core()
{
    qmlRegisterType<Utils>("utils", 1, 0, "Utils");

    engine = new QQmlApplicationEngine();
    engine->load("qrc:/widgets/MainWindow.qml");
}

Core::~Core()
{
    engine->deleteLater();
}
