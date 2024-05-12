#pragma once

#include <QObject>

namespace Model
{
    class File : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString name READ getName CONSTANT)
        Q_PROPERTY(QString size READ getSizeName CONSTANT)

    public:
        explicit File(const QString &name, long long int size, QObject *parent = nullptr);
        ~File() = default;

        const QString &getName() const;
        QString getSizeName() const;

    private:
        const QString name;
        const long long int size;
    };

    inline const QString &File::getName() const
    {
        return name;
    }
} // namespace Model
