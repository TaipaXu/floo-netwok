#include "./settings.hpp"
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

namespace Persistence
{
    Settings::Settings(QObject *parent)
        : QObject(parent), Persistence()
    {
        settings = new QSettings(getSettingsFilePath(), QSettings::IniFormat);
        readSettings();
    }

    Settings::~Settings()
    {
        writeSettings();
        settings->deleteLater();
    }

    void Settings::setDownloadPath(const QUrl &url)
    {
        downloadPath = url.toLocalFile();
        emit downloadPathChanged();
        settings->setValue("downloadPath", downloadPath);
    }

    QString Settings::getSettingsFilePath() const
    {
        const QString fileName = QStringLiteral("%1.ini").arg("settings");
        const QString filePath = Persistence::getSettingsFilePath(fileName);

        return filePath;
    }

    void Settings::readSettings()
    {
        downloadPath = settings->value("downloadPath", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
        emit downloadPathChanged();
    }

    void Settings::writeSettings()
    {
        settings->sync();
    }
} // namespace Persistence
