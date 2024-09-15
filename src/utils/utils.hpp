#pragma once

#include <QObject>
#include <QString>

class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);
    ~Utils() = default;

    Q_INVOKABLE static QStringList getLocalIPv4Addresses();

    static bool isLocalAddress(const QString &address);

    Q_INVOKABLE static bool isIPv4Address(const QString &address);

    static QString getReadableSize(long long int size);

    Q_INVOKABLE static void copyToClipboard(const QString &text);

    Q_INVOKABLE static void openMyGithubPage();

    Q_INVOKABLE static void openThisProjectPage();

    Q_INVOKABLE static void openThisProjectIssuesPage();

    Q_INVOKABLE static void openFileDirectory(const QString &filePath);
};
