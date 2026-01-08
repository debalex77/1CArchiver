/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void checkForUpdates(const QString &currentVersion); /** verificam versiunea noua a app */

signals:
    void updateAvailable(const QString &newVersion); /** versiunea noua este disponibila */
    void noUpdate();
    void error(const QString &msg);

private slots:
    void onVersionReply(QNetworkReply *reply);

private:
    QNetworkAccessManager m_net;
    QString m_currentVersion;

};

#endif // UPDATECHECKER_H
