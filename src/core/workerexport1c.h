/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef WORKEREXPORT1C_H
#define WORKEREXPORT1C_H

#include <QObject>
#include <QProcess>

/*
 * WorkerExport1C
 * --------------
 * Exportul bazei de date prin platforma 1C:
 *   1cv8.exe DESIGNER /F"<folder>" | /S"<server>\<baza>"
 *            /N"<user>" /P"<parola>" /DumpIB"<fisier.dt>" /Out"<log>"
 *
 * QProcess este asincron -> nu e necesar un QThread separat.
 */
class WorkerExport1C : public QObject
{
    Q_OBJECT
public:
    explicit WorkerExport1C(QObject *parent = nullptr);
    ~WorkerExport1C();

    /** cauta config-ul export_1c pentru baza: <nume>.json, apoi global_all_bases.json */
    static QString findConfig(const QString &dbName, const QString &dbFolder);

    /** true daca config-ul descrie o baza de server 1C (nu e necesar 1Cv8.1CD local) */
    static bool isServerConfig(const QString &configPath);

    void setConfigFile(const QString &path);
    void setDbFolder(const QString &folder);  /** folosit cand config-ul e global sau dbPath lipseste */
    void setOutputDt(const QString &dtPath);

    void process();
    void cancel();

signals:
    void log(const QString &);
    void finished(bool ok,
                  const QString &dtPath,
                  const QString &error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);

private:
    QString readOutLog() const;
    void finish(bool ok, const QString &error);

private:
    QProcess *m_proc = nullptr;
    bool m_done = false;

    QString m_configError;
    QString m_platform;
    QString m_typeDB;
    QString m_server;
    QString m_database;
    QString m_dbPath;
    QString m_user;
    QString m_pass;
    bool m_global = false;

    QString m_dbFolder;
    QString m_outputDt;
    QString m_outLog;
};

#endif // WORKEREXPORT1C_H
