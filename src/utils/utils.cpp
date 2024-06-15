#include "./utils.hpp"
#include <sstream>
#include <iomanip>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkInterface>
#include <QUuid>
#include <QClipboard>
#include "appConfig.hpp"

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

QStringList Utils::getLocalIPv4Addresses()
{
    QStringList addresses;
    for (auto &&interface : QNetworkInterface::allInterfaces())
    {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            for (auto &&address : interface.addressEntries())
            {
                if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    addresses.append(address.ip().toString());
                }
            }
        }
    }
    return addresses;
}

bool Utils::isLocalAddress(const QString &address)
{
    QStringList addresses = getLocalIPv4Addresses();
    for (auto &&localAddress : addresses)
    {
        if (localAddress == address)
        {
            return true;
        }
    }
    return false;
}

bool Utils::isIPv4Address(const QString &address)
{
    QHostAddress hostAddress(address);
    return hostAddress.protocol() == QAbstractSocket::IPv4Protocol;
}

QString Utils::getReadableSize(long long int size)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (size < 1024)
    {
        ss << size << "B";
    }
    else if (size < 1024 * 1024)
    {
        ss << size / 1024.0 << "KB";
    }
    else if (size < 1024 * 1024 * 1024)
    {
        ss << size / 1024.0 / 1024 << "MB";
    }
    else
    {
        ss << size / 1024.0 / 1024 / 1024 << "GB";
    }
    return QString::fromStdString(ss.str());
}

void Utils::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void Utils::openMyGithubPage()
{
    QDesktopServices::openUrl(QUrl(ORGANIZATION_DOMAIN));
}

void Utils::openThisProjectPage()
{
    QDesktopServices::openUrl(QUrl(PROJECT_DOMAIN));
}

void Utils::openThisProjectIssuesPage()
{
    QDesktopServices::openUrl(QUrl(PROJECT_ISSUES_DOMAIN));
}
