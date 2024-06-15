#include "./file.hpp"
#include "utils/utils.hpp"

namespace Model
{
    File::File(const QString &id, const QString &name, long long int size, QObject *parent)
        : QObject(parent), id{id}, name{name}, size{size}
    {
    }

    QString File::getSizeName() const
    {
        return Utils::getReadableSize(size);
    }

    QJsonObject toJson(const File *file)
    {
        QJsonObject json{
            {"id", file->getId()},
            {"name", file->getName()},
            {"size", file->getSize()},
        };
        return json;
    }

    QJsonArray toJson(const QList<Model::File *> &files)
    {
        QJsonArray json;
        for (const auto &file : files)
        {
            json.append(toJson(file));
        }
        return json;
    }
} // namespace Model
