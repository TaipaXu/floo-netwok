#include "./file.hpp"
#include "utils/utils.hpp"

namespace Model
{
    File::File(const QString &name, long long int size, QObject *parent)
        : QObject(parent), name{name}, size{size}
    {
    }

    QString File::getSizeName() const
    {
        return Utils::getReadableSize(size);
    }
} // namespace Model
