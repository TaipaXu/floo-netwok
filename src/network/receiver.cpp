#include "./receiver.hpp"
#include <QUuid>

namespace Network
{
    Receiver::Receiver(QObject *parent)
        : QObject(parent),
          id{QUuid::createUuid().toString(QUuid::WithoutBraces)}
    {
    }
} // namespace Network
