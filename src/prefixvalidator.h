#ifndef PREFIXVALIDATOR_H
#define PREFIXVALIDATOR_H

#include <QDebug>

#include <QValidator>

class PrefixValidator : public QValidator
{
    Q_OBJECT

public:
    PrefixValidator(QString prefix, QObject *parent) : QValidator(parent)
    {
        this->prefix = prefix;
    }

    State validate(QString &input, int &pos) const
    {
        Q_UNUSED(pos);

        if (input.startsWith(prefix))
            return input.length() == prefix.length() ? Intermediate : Acceptable;
        else
            return Invalid;
    }

private:
    QString prefix;
};

#endif // PREFIXVALIDATOR_H
