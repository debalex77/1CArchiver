/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef WORKERDUMPIB_H
#define WORKERDUMPIB_H

#include <QObject>
#include <QProcess>

/*
 * Exportul bazei 1C in format .dt cu ajutorul platformei:
 *   1cv8.exe DESIGNER /F"folder" | /S"server\baza" /DumpIB"fisier.dt"
 */
class WorkerDumpIB : public QObject
{
    Q_OBJECT
public:
    explicit WorkerDumpIB(QObject *parent = nullptr);
    ~WorkerDumpIB();

    void setConfigFile(const QString &path);
    void setDbFolder(const QString &folder); /** baza de tip fisier -> /F; gol -> baza de server /S */
    void setOutputDt(const QString &dtPath);

    void process();
    void cancel();

    /** cea mai noua versiune instalata: Program Files\1cv8\<ver>\bin\1cv8.exe */
    static QString findPlatform();

signals:
    void log(const QString &);
    void finished(bool ok,
                  const QString &dtPath,
                  const QString &error);

private slots:
    void onProcessFinished(int exitCode,
                           QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);

private:
    QString readOutLog() const;
    void finish(bool ok, const QString &error);

private:
    QProcess *m_proc = nullptr;

    QString m_platform;
    QString m_server1c;
    QString m_infobase;
    QString m_user;
    QString m_pass;
    QString m_dbFolder;
    QString m_outputDt;
    QString m_outLog;
    QString m_configError; /** eroare la citirea config -> raportata in process() */
    bool    m_done = false;
};

#endif // WORKERDUMPIB_H
