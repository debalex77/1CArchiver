/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef PLUGINCONFIGDIALOG_H
#define PLUGINCONFIGDIALOG_H

#include <QDialog>
#include <QDialog>
#include <QJsonObject>

class DynamicPluginForm;

class PluginConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PluginConfigDialog(const QString &pluginId,
                                const QString &configFile,
                                QWidget *parent = nullptr);

signals:
    void onAddedDatabase(const QVariantMap &dbInfo);

private slots:
    void onAccept();

private:
    void loadSchema();
    void loadConfig();
    void saveConfig();

    QString schemaPath() const;

    QString m_pluginId;
    QString m_configFile;

    QJsonObject m_schema;
    QJsonObject m_config;

    DynamicPluginForm *m_form = nullptr;
};

#endif // PLUGINCONFIGDIALOG_H
