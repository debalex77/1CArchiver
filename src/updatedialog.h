/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardPaths>
#include <QFile>
#include <QProcess>

class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDialog(const QString &version, QWidget *parent = nullptr);

private slots:
    void startDownload();
    void onDownloadProgress(qint64 received, qint64 total);
    void onFinished();

private:
    QString m_version;
    QNetworkAccessManager m_net;
    QNetworkReply *m_reply = nullptr;

    QProgressBar *m_progress;
    QPushButton  *m_btn;
};

#endif // UPDATEDIALOG_H
