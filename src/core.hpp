#pragma once

#include <qobjectdefs.h>

QT_BEGIN_NAMESPACE
class QQmlApplicationEngine;
QT_END_NAMESPACE

class Core
{
public:
    Core();
    ~Core();

private:
    QQmlApplicationEngine *engine;
};
