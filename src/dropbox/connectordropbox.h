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

#include <QObject>
#include <QString>

class DropboxUploader;
class DropboxOAuth2_PKCE;

/*
 * ConnectorDropbox
 * -----------------
 * Interfață între aplicația principală (UI) și sistemul Dropbox:
 *   - gestionează tokenurile în QSettings
 *   - oferă metodă de test upload
 *   - oferă acces simplu la login (OAuth2 PKCE)
 */
class ConnectorDropbox : public QObject
{
    Q_OBJECT

public:
    explicit ConnectorDropbox(QObject *parent = nullptr);

    QString accessToken() const;
    QString refreshToken() const;

    /** Salvează tokenuri în QSettings */
    void saveTokens(const QString &at, const QString &rt);

    /** Test upload (folosit de UI) */
    void testUpload();

signals:
    /** Emit când testul de upload s-a terminat */
    void testFinished(bool ok, const QString &msg);

private slots:
    void onTestFinished(bool ok, const QString &msg);

private:
    QString m_accessToken;
    QString m_refreshToken;
};
