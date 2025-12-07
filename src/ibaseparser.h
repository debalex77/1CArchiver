#ifndef IBASEPARSER_H
#define IBASEPARSER_H

#include <QString>
#include <QList>
#include "IBASEEntry.h"

class IBASEParser
{
public:
    static QList<IBASEEntry> parse(const QString &filePath);
};

#endif // IBASEPARSER_H