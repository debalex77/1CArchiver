/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GLOBALS_H
#define GLOBALS_H
#include <QString>

#pragma once

namespace globals {

    extern bool isAutorun;
    extern bool isDark;
    extern QString currentLang;

    extern bool backupExtFiles;
    extern bool createFileSHA256;

    extern bool questionCloseApp;

    extern bool setArchivePassword;
    extern QString archivePassword;

    extern bool syncDropbox;
    extern bool activate_syncDropbox;
    extern QString loginSuccesDropbox;

    extern bool syncGoogleDrive;
    extern bool activate_syncGoogleDrive;
    extern QString loginSuccesGoogleDrive;

    extern bool deleteArchives;
    extern int lastNrDay;
}

#

#endif // GLOBALS_H
