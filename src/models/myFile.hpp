#pragma once

#include "./file.hpp"

namespace Model
{
    class MyFile : public File
    {
        Q_OBJECT

        Q_PROPERTY(QString path READ getPath CONSTANT)

    public:
        MyFile(const QString &name, long long int size, const QString &path, QObject *parent = nullptr);
        ~MyFile() = default;

        const QString &getPath() const;

    private:
        const QString path;
    };

    inline const QString &MyFile::getPath() const
    {
        return path;
    }
} // namespace Model
