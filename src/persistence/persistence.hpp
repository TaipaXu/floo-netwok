#pragma once

#include <QString>

class QSettings;

namespace Persistence
{
    class Persistence
    {
    public:
        Persistence();
        virtual ~Persistence() = default;

    protected:
        QString getSettingsDirPath() const;
        QString getSettingsFilePath(const QString &fileName) const;
        QString getDataDirPath() const;
        QString getDataFilePath(const QString &fileName) const;
        virtual void readSettings() = 0;
        virtual void writeSettings() = 0;

    protected:
        QString dataDirPath;
    };
} // namespace Persistence
