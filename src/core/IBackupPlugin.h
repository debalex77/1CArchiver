/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QObject>
#include <QVariantMap>

/*
 * IBackupPlugin
 * -------------
 * Interfată COMUNA pentru toate pluginurile de backup.
 * Comunicarea se face EXCLUSIV prin semnale.
 */
class IBackupPlugin
{
public:
    virtual ~IBackupPlugin() = default;

    /** porneste procesul de backup */
    virtual void start(const QVariantMap &config) = 0;

    /** anuleaza backup-ul in curs (dacă este posibil) */
    virtual void cancel() = 0;

signals:
    virtual void log(const QString &message) = 0;
    virtual void progress(int percent) = 0;
    virtual void finished( bool success,
                          const QString &outputPath,
                          const QString &errorMessage
                          ) = 0;
};

/*
 * IID-ul Qt pentru QPluginLoader.
 * Trebuie sa fie IDENTIC in aplicatie si plugin DLL.
 */
#define IBackupPlugin_iid "com.oxvalprim.1carchiver.backupplugin/1.0"
Q_DECLARE_INTERFACE(IBackupPlugin, IBackupPlugin_iid)
