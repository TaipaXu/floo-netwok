#pragma once

#include <QObject>
#include <QString>

class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);
    ~Utils() = default;

    Q_INVOKABLE static QStringList getLocalAddresses();

    static QString getReadableSize(long long int size);
};
