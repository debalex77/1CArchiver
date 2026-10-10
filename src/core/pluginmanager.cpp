#include "pluginmanager.h"
#include "src/globals.h"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

PluginManager::PluginManager()
{
    /** starea implicită dezactivata */
    m_plugins["mssql"]    = false;
    m_plugins["rsync"]    = false;
    m_plugins["onedrive"] = false;
    m_plugins["export_1c"] = false;
    m_plugins["telegram"]  = false;
}

void PluginManager::load()
{
    QFile file(settingsPath());
    if (!file.exists())
        return; /** prima lansare */

    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();
    QJsonArray arr_plugins = root["plugins"].toArray();
    for (const QJsonValue &val : std::as_const(arr_plugins)) {
        QJsonObject o = val.toObject();
        for (auto it = o.begin(); it != o.end(); ++it) {
            m_plugins[it.key()] = it.value().toBool(false);
        }
    }

    /** setam variabile globale */
    globals::pl_mssql    = m_plugins["mssql"];
    globals::pl_rsync    = m_plugins["rsync"];
    globals::pl_onedrive = m_plugins["onedrive"];
    globals::pl_export1c = m_plugins["export_1c"];
    globals::pl_telegram = m_plugins["telegram"];
}

void PluginManager::save() const
{
    QJsonArray pluginsArray;

    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        QJsonObject o;
        o[it.key()] = it.value();
        pluginsArray.append(o);
    }

    /** pastram restul setarilor (paths_db, backupFolder etc.) - modificam doar "plugins" */
    QJsonObject root;
    {
        QFile in(settingsPath());
        if (in.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(in.readAll());
            if (doc.isObject())
                root = doc.object();
        }
    }
    root["plugins"] = pluginsArray;

    QFile file(settingsPath());
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(root)
                   .toJson(QJsonDocument::Indented));
}

bool PluginManager::isEnabled(const QString &pluginId) const
{
    return m_plugins.value(pluginId, false);
}

void PluginManager::setEnabled(const QString &pluginId, bool enabled)
{
    m_plugins[pluginId] = enabled;
    save();
}

QStringList PluginManager::enabledPlugins() const
{
    QStringList list;
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if (it.value())
            list << it.key();
    }
    return list;
}

QString PluginManager::settingsPath() const
{
    QString basePath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    /** Creează %APPDATA%/1CArchiver dacă nu exista */
    QDir().mkpath(basePath);
    return basePath + "/settings.json";
}
