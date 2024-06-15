#include "./myFile.hpp"

namespace Model
{
    MyFile::MyFile(const QString &name, long long int size, const QString &path, QObject *parent)
        : File(path, name, size, parent), path{path}
    {
    }

    QJsonArray toJson(const QList<Model::MyFile *> &files)
    {
        QJsonArray json;
        for (const auto &file : files)
        {
            json.append(toJson(file));
        }
        return json;
    }
} // namespace Model
