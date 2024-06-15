#include "./persistence.hpp"
#include <QStandardPaths>
#include "appConfig.hpp"

namespace Persistence
{
    Persistence::Persistence()
    {
        dataDirPath = getDataDirPath();
    }

    QString Persistence::getSettingsDirPath() const
    {
        const QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        return path;
    }

    QString Persistence::getSettingsFilePath(const QString &fileName) const
    {
        const QString dirPath = getSettingsDirPath();
        QString filePath = QStringLiteral("%1/%2/%3").arg(dirPath).arg(PROJECT_NAME).arg(fileName);

        return filePath;
    }

    QString Persistence::getDataDirPath() const
    {
        const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        return path;
    }

    QString Persistence::getDataFilePath(const QString &fileName) const
    {
        const QString dirPath = getDataDirPath();
        QString filePath = QStringLiteral("%1/%2").arg(dirPath).arg(fileName);

        return filePath;
    }
} // namespace Persistence
