/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef DROPBOXHEALTHCHECKER_H
#define DROPBOXHEALTHCHECKER_H

#include <QNetworkAccessManager>
#include <QObject>

/*
 * Clasa responsabila pentru:
 *   - verificare autorizatiei la Dropbox:
 *     - connected
 *     - authorization required
 */
class DropboxHealthChecker : public QObject
{
    Q_OBJECT
public:
    explicit DropboxHealthChecker(QObject *parent = nullptr);

    void check(const QString &accessToken);

signals:
    void connected();
    void authorizationRequired();

private:
    QNetworkAccessManager m_net;
};

#endif // DROPBOXHEALTHCHECKER_H
