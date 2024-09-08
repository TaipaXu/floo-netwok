#include "./sender.hpp"
#include <QUuid>

namespace Network
{
    Sender::Sender(QObject *parent)
        : QObject(parent),
          id{QUuid::createUuid().toString(QUuid::WithoutBraces)}
    {
    }
} // namespace Network
