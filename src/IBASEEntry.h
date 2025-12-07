#ifndef IBASEENTRY_H
#define IBASEENTRY_H

#include <QString>

struct IBASEEntry
{
    QString sectionName;   // [ClinicBase]
    QString displayName;   // Name=
    QString filePath;      // Folder BD (File="...")
};

#endif // IBASEENTRY_H