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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QPointer>

class DropboxOAuth2_PKCE;

enum class UploadState {
    Idle,
    Uploading,
    RefreshingToken,
    Failed
};

/*
 * Clasa responsabila pentru:
 *   - upload fișiere prin Dropbox API v2
 *   - gestiune access_token expirat
 *   - retry automat după refresh
 */

class DropboxUploader : public QObject
{
    Q_OBJECT

public:
    explicit DropboxUploader(const QString &accessToken,
                             const QString &refreshToken,
                             QObject *parent = nullptr);

    // Pornește uploadul
    void uploadFile(const QString &localPath, const QString &remotePath);
    void abort();

signals:
    void uploadProgress(qint64 sent, qint64 total);
    void uploadFinished(bool ok, const QString &msg);
    void authError(const QString &msg);

private slots:
    void onUploadReply();
    void onUploadProgress(qint64 sent, qint64 total);

    // După refresh token
    void onRefreshSuccess();
    void onRefreshFail(const QString &reason);

private:
    void startUpload();      // metoda internă pentru upload
    void tryRefreshToken();  // pornește refresh token

private:
    QString m_localPath;
    QString m_remotePath;

    QString m_accessToken;
    QString m_refreshToken;

    QFile m_file;
    QNetworkAccessManager m_net;
    QPointer<QNetworkReply> m_reply;
    DropboxOAuth2_PKCE *m_oauth = nullptr;

    bool m_retryAfterRefresh = false;

    UploadState m_state = UploadState::Idle;
};
