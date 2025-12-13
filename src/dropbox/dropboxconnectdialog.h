/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include <QDialog>

class ConnectorDropbox;
class DropboxOAuth2_PKCE;

namespace Ui {
class DropboxConnectDialog;
}

/*
 * DropboxConnectDialog
 * ---------------------
 * Dialog UI care permite:
 *   - Login cu Dropbox (OAuth2 PKCE)
 *   - Test upload
 *   - Afișare mesaje pentru utilizator
 */
class DropboxConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DropboxConnectDialog(QWidget *parent = nullptr);
    ~DropboxConnectDialog();

signals:
    // Logarea cu succes la Dropbox
    void loginSuccesDropbox();

private slots:
    // Slot-le
    void onLoginClicked();
    void onLoginSuccess();
    void onLoginFailed(const QString &msg);

    // Test
    void onTestClicked();
    void onTestFinished(bool ok, const QString &msg);

private:
    Ui::DropboxConnectDialog *ui;
    ConnectorDropbox *m_connector;
};
