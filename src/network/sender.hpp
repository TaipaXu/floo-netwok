#pragma once

#include <QObject>

namespace Network
{
    class Sender : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(Type type READ getType CONSTANT)
        Q_PROPERTY(QString id MEMBER id CONSTANT)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        Q_PROPERTY(QString size READ getSize NOTIFY sizeChanged)

    public:
        enum class Type
        {
            Tcp,
            Http
        };
        Q_ENUM(Type)

    public:
        Sender(QObject *parent = nullptr);
        ~Sender() = default;

    signals:
        void nameChanged() const;
        void sizeChanged() const;

    protected:
        virtual Type getType() const
        {
            return Type::Tcp;
        };
        virtual QString getName() const
        {
            return QStringLiteral();
        };
        virtual QString getSize() const
        {
            return QStringLiteral();
        };

    private:
        const QString id;
    };
} // namespace Network
