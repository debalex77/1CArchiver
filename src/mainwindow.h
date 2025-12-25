/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QProcess>
#include <QTimer>
#include <QVector>
#include <QMovie>
#include <QComboBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QEventLoop>
#include <QTranslator>
#include <QToolButton>

#include <src/dropbox/dropboxuploader.h>

#include "ibaseentry.h"
#include "switchbutton.h"
#include "appsettings.h"

namespace bit7z {
    class Bit7zLibrary;
    class BitFileCompressor;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void startBackup(); // functia externa pu "--autorun" vezi in main.cpp

signals:
    void jobFinishedSignal(bool ok);
    void allJobsFinished();           // pu "--autorun" vezi in main.cpp

private slots:
    void onSelectAll();
    void onChooseDirWithDb();
    void onChooseBackupFolder();
    void onStartArchive();
    void switchLanguage(const QString& lang);
    void applyTheme();
    void clickedAbortDropbox();

    void onTableContextMenu(const QPoint &pos);
    void clearAllRows();
    void removeCurrentRow();
    void autoDetectPaths1C();

    void saveLogToFile();
    void checkForUpdates();

private:
    struct BackupJob {
        int row;
        QString dbName;
        QString dbFolder;
        QString file1CD;
        QString archivePath;

        QMovie *spinner = nullptr;
        QLabel *statusLabel = nullptr;
        bool archiveWholeFolder;
    };

    /** UI */
    QTableWidget *table = nullptr;

    QPushButton *btnSelectAll    = nullptr;
    QPushButton *btnWithDb       = nullptr;
    QPushButton *btnFolder       = nullptr;
    QPushButton *btnArchive      = nullptr;
    QPushButton *btnGenerateTask = nullptr;
    QToolButton *btnSettings     = nullptr;
    QToolButton *btnAbout        = nullptr;

    QLabel *currentStatus     = nullptr;
    QProgressBar *progressBar = nullptr;

    QLabel *currentStatusDropbox     = nullptr;
    QProgressBar *progressBarDropbox = nullptr;
    QPushButton *btnAbortDropbox     = nullptr;

    QTextEdit *logBox         = nullptr;

    QComboBox *comboCompression = nullptr;

    /** Data folder backup & path to bases */
    QString backupFolder;
    QVector<IBASEEntry> bases;

    /** 7-Zip process */
    QTimer *progressTimer      = nullptr;
    QString currentArchivePath = nullptr;
    qint64 sourceFileSize = 0;

    /** Job queue compress, archive */
    QVector<BackupJob> jobs;
    int currentJob = -1;

    QString settingsFilePath;

    /** lib for bit7z */
    bit7z::Bit7zLibrary* m_lib = nullptr;
    bit7z::BitFileCompressor* m_compressor = nullptr;

    qint64 m_currentTotalBytes = 0;   /** dimensiunea totală a 1Cv8.1CD pentru job-ul curent */

    QLabel *lblCompression;
    QTranslator translator;
    QString currentLang;

    /** button switch - theme & lang */
    SwitchButton* themeSwitch;
    SwitchButton* lblSwitch;
    QLabel* lblLang;
    QLabel* themeLabel;

    AppSettings* app_settings;

    DropboxUploader *m_dbxUploader = nullptr;

    /** for dropbox */
    bool m_waitingForDropbox = false;
    bool m_dropboxStartupCheckRunning = false;

    bool m_checkedUpdates = false;

private:
    void check7ZipInstallation();

    QStringList find1CDBaseFolders(const QString& rootDir);

    void log(const QString &msg);

    QString buildArchiveName(const QString &dbName) const;
    void startNextJob();
    void updateRowStatusIcon(int row, bool ok);
    QString get7zPath() const;

    void startDropboxUpload(const QString &localPath, const QString &fileSHA256 = QString());

    bool createSha256File(const QString& filePath);

    void setPropertyVisible();

    QWidget* createCheckBoxWidget(QWidget *parent);
    void loadSettings();
    void saveSettings();

    void retranslateUi();

    void checkDropboxAtStartup();
    void setDropboxConnected();
    void setDropboxAuthRequired();

    void cleanupOldArchives();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

};

#endif // MAINWINDOW_H
