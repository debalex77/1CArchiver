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

/**
 * Criptarea parolelor salvate in fisierele JSON (settings.json, config-uri plugin).
 *
 *   - Windows DPAPI (CryptProtectData, scope CurrentUser) -> parola poate fi
 *     decriptata doar de acelasi utilizator Windows, pe acelasi calculator.
 *     Format salvat: "dpapi:<base64>"
 *   - formatul vechi (XOR + base64, fara prefix) este citit in continuare;
 *     la urmatoarea salvare parola se re-cripteaza automat cu DPAPI.
 *
 * La esec (alt utilizator / alt PC / date corupte) decryptPassword() intoarce "".
 */
QString encryptPassword(const QString& pass);
QString decryptPassword(const QString& encoded);
