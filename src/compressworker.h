/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <QObject>
#include <QString>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitfilecompressor.hpp>
#include <bit7z/bitformat.hpp>

/*
 * Clasa responsabila pentru:
 *   - comprimarea bd 1C (.7z)
 */
class CompressWorker : public QObject {
    Q_OBJECT
public:
    CompressWorker(QString input,
                   QString output,
                   int level,
                   bool isFolder,
                   uint64_t totalSize,
                   const QString& password = QString());

signals:
    void progress(int pct);
    void finished(bool ok, QString output);
    void error(QString msg);
    void backupCreated(const QString &filePath);

public slots:
    void process();

private:
    QString  m_input;
    QString  m_output;
    int      m_level;
    bool     m_isFolder;
    uint64_t m_totalSize;
    QString  m_password;
};


