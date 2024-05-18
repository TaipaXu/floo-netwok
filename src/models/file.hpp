#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

namespace Model
{
    class File : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString id READ getId CONSTANT)
        Q_PROPERTY(QString name READ getName CONSTANT)
        Q_PROPERTY(QString size READ getSizeName CONSTANT)

    public:
        explicit File(const QString &id, const QString &name, long long int size, QObject *parent = nullptr);
        ~File() = default;

        const QString &getId() const;
        const QString &getName() const;
        int getSize() const;
        QString getSizeName() const;

    private:
        const QString id;
        const QString name;
        const long long int size;
    };

    inline const QString &File::getId() const
    {
        return id;
    }

    inline int File::getSize() const
    {
        return size;
    }

    inline const QString &File::getName() const
    {
        return name;
    }

    QJsonObject toJson(const File *file);
    QJsonArray toJson(const QList<Model::File *> &files);
} // namespace Model
