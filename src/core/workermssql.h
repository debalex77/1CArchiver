/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef WORKERMSSQL_H
#define WORKERMSSQL_H

#include <QObject>
#include <QProcess>
#include <QSqlDatabase>
#include <QTimer>

class WorkerMssql : public QObject
{
    Q_OBJECT
public:
    explicit WorkerMssql(QObject *parent = nullptr);
    ~WorkerMssql();

    void setConfigFile(const QString &path);
    void setOutputBak(const QString &bakPath);

    void process();
    void cancel();

signals:
    void log(const QString &);
    void progress(int);
    void finished(bool ok,
                  const QString &bakPath,
                  const QString &error);

private slots:
    void onSqlcmdFinished(int exitCode,
                          QProcess::ExitStatus status);
    void pollProgress();

private:
    bool openProgressDb();
    void closeProgressDb();

private:
    QProcess *m_proc = nullptr;
    QTimer   *m_timer = nullptr;
    QSqlDatabase m_db;

    QString m_auth;
    QString m_user;
    QString m_pass;
    QString m_server;
    QString m_database;
    QString m_outputBak;
};

#endif // WORKERMSSQL_H
