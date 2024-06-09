#pragma once

#include <QObject>
#include "./persistence.hpp"

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace Persistence
{
    class Settings : public QObject, public Persistence
    {
        Q_OBJECT

        Q_PROPERTY(QString downloadPath READ getDownloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)

    public:
        Settings(QObject *parent = nullptr);
        ~Settings();

        QString getDownloadPath() const;
        void setDownloadPath(const QUrl &url);

    signals:
        void downloadPathChanged();

    private:
        QString getSettingsFilePath() const;
        void readSettings() override;
        void writeSettings() override;

    private:
        QSettings *settings;
        QString downloadPath;
    };

    inline QString Settings::getDownloadPath() const
    {
        return downloadPath;
    }
} // namespace Persistence
