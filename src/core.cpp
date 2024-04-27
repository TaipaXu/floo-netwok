#include "./core.hpp"
#include <QQmlApplicationEngine>

Core::Core()
{
    engine = new QQmlApplicationEngine();
    engine->load("qrc:/widgets/MainWindow.qml");
}

Core::~Core()
{
    engine->deleteLater();
}
