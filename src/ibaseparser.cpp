#include "ibaseparser.h"
#include <QFile>
#include <QTextStream>
#include <QDir>

QList<IBASEEntry> IBASEParser::parse(const QString &filePath)
{
    QList<IBASEEntry> list;

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return list;

    QTextStream ts(&f);

    QString section;
    QString name;
    QString filePathBD;

    auto save = [&]() {
        if (!section.isEmpty() && !filePathBD.isEmpty()) {
            IBASEEntry e;
            e.sectionName = section;
            e.displayName = name.isEmpty() ? section : name;
            e.filePath = QDir::toNativeSeparators(filePathBD);
            list.append(e);
        }
    };

    while (!ts.atEnd())
    {
        QString line = ts.readLine().trimmed();

        if (line.startsWith("[") && line.endsWith("]"))
        {
            // salvăm secțiunea anterioară
            save();

            section = line.mid(1, line.length() - 2).trimmed();
            name.clear();
            filePathBD.clear();
            continue;
        }

        // găsim Connect=File=
        if (line.startsWith("Connect=File=", Qt::CaseInsensitive))
        {
            QString value = line.mid(QString("Connect=File=").length());

            // Cu ghilimele
            if (value.startsWith("\"")) {
                int end = value.indexOf("\"", 1);
                if (end > 1)
                    filePathBD = value.mid(1, end - 1);
            }
            else {
                // Fără ghilimele → până la ;
                int end = value.indexOf(";");
                if (end < 0) end = value.length();
                filePathBD = value.left(end).trimmed();
            }

            continue;
        }

        // nume BD
        if (line.startsWith("Name=", Qt::CaseInsensitive))
        {
            name = line.mid(5).trimmed();
        }
    }

    // ultima secțiune
    save();

    return list;
}


