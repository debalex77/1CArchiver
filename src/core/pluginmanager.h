/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QMap>

/*
 * PluginManager
 * --------------
 * Responsabil DOAR de:
 *  - starea pluginurilor (ON/OFF)
 *  - persistarea în settings.json
 *    "plugins": {
 *      "mssql": true,
 *      "rsync": false,
 *      "onedrive": false
 *     }
 */
class PluginManager
{
public:
    PluginManager();

    void load();       /** Încarcă starea pluginurilor din settings.json */
    void save() const; /** Salvează starea pluginurilor în settings.json */

    bool isEnabled(const QString &pluginId) const;
    void setEnabled(const QString &pluginId, bool enabled);

    QStringList enabledPlugins() const; /** Returnează lista pluginurilor active */

private:

    QString settingsPath() const; /** Calea completă către settings.json */

    QMap<QString, bool> m_plugins; /** Map intern: pluginId -> enabled */
};

#endif // PLUGINMANAGER_H
