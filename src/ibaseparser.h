/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef IBASEPARSER_H
#define IBASEPARSER_H

#include <QString>
#include <QList>
#include "IBASEEntry.h"

/*
 * Clasa responsabila pentru:
 *   - determinarea/parsarea denumirei bd, drumului spre bd din fisierul:
 *     C:\Users\user\AppData\Roaming\1C\1CEStart\ibases.v8i
 */
class IBASEParser
{
public:
    static QList<IBASEEntry> parse(const QString &filePath);
};

#endif // IBASEPARSER_H
