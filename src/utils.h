/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once
#include <QString>

/*
 * Parolele salvate (arhiva, MSSQL) - implementare in utils.cpp
 *   - format nou:  "dpapi:<base64>"  (Windows DPAPI, legat de utilizatorul Windows)
 *   - format vechi: base64(XOR)      (v1.8 si mai vechi) - doar citire, pentru migrare
 */

/** criptare DPAPI; "" -> "" ; la eroare -> "" */
QString encryptPassword(const QString& pass);

/** "dpapi:..." -> DPAPI ; altfel -> formatul vechi XOR ; la eroare DPAPI -> "" */
QString decryptPassword(const QString& encoded);

/** true daca valoarea salvata e in formatul vechi (XOR) si trebuie recriptata */
bool isLegacyPassword(const QString& encoded);

/** nume de fisier valid in Windows (ex. ООО "Ромашка", SRV\SQLEXPRESS) */
inline QString safeFileName(const QString& name) {
    QString s = name;
    for (QChar &ch : s) {
        if (ch.unicode() < 0x20 || QStringLiteral("<>:\"/\\|?*").contains(ch))
            ch = QLatin1Char('_');
    }

    /** Windows nu accepta punct/spatiu la sfarsitul numelui */
    while (s.endsWith(QLatin1Char('.')) || s.endsWith(QLatin1Char(' ')))
        s.chop(1);

    return s.isEmpty() ? QStringLiteral("db") : s;
}

