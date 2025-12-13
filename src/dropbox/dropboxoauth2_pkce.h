/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 /*****************************************************************************/

#pragma once

#include <QObject>
#include <QTcpServer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>

/*
 * Clasa implementează:
 *  - OAuth2 PKCE complet pentru Dropbox
 *  - Fallback pe porturi 53682–53686
 *  - Generarea code_verifier, code_challenge (SHA-256)
 *  - Pornirea browser-ului pentru autentificare
 *  - Server HTTP local pentru redirect + captarea authorization_code
 *  - Schimb authorization_code → access_token + refresh_token
 *  - Refresh automat de access_token (grant_type=refresh_token)
 */

class DropboxOAuth2_PKCE : public QObject
{
    Q_OBJECT

public:
    explicit DropboxOAuth2_PKCE(QObject *parent = nullptr);

    // Pornește fluxul OAuth2 PKCE (deschide browser-ul)
    void startLoginFlow();

    // Reînnoire access_token folosind refresh_token
    void refreshAccessToken();

    // Accesorii
    QString accessToken() const { return m_accessToken; }
    QString refreshToken() const { return m_refreshToken; }

signals:
    // Autentificarea a reușit
    void loginSucceeded();
    // Autentificarea a eșuat
    void loginFailed(const QString &reason);

    // Refresh token success / fail
    void refreshSucceeded();
    void refreshFailed(const QString &reason);

private slots:
    // Server HTTP – conexiune nouă de la browser
    void onIncomingConnection();

    // Răspunsul după schimbul authorization_code → tokenuri
    void onTokenReply();

    // Răspunsul la refresh token
    void onRefreshReply();

private:
    // Functii locale
    QString generateCodeVerifier();
    QString generateCodeChallenge(const QString &verifier);
    QString base64UrlEncode(const QByteArray &data);

    bool startCallbackServer();        // încearcă porturile 53682–53686
    void exchangeCodeForTokens(const QString &code);

private:
    QTcpServer m_server;
    QNetworkAccessManager m_net;

    QString m_codeVerifier;
    QString m_codeChallenge;

    QString m_accessToken;
    QString m_refreshToken;

    int m_callbackPort = -1;

    // Lista porturilor fallback
    const QList<int> m_ports = {53682, 53683, 53684, 53685, 53686};

    // APP KEY
    const QString CLIENT_ID = "gfkrwtxtibh08xp";
};


