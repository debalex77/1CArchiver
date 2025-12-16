/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "switchbutton.h"
#include "globals.h"
#include "lineeditpassword.h"

#include <QEvent>
#include <QDialog>
#include <QLabel>
#include <QObject>

/*
 * Clasa responsabila pentru:
 *   - setarea variabilelor globale
 *   - setarea parolei arhivei
 *   - setarea crearii fisierelor SHA256
 *   - sincronizarea in Dropbox si GoogleDrive
 */
class AppSettings : public QDialog
{
    Q_OBJECT
public:
    explicit AppSettings(QWidget* parent = nullptr);
    ~AppSettings();

    void setChecked(bool on);
    bool isChecked() const;

signals:
    void onActivateDropbox();

private:
    QString highlightColor;

    QLabel* lbl_setArchivePassword;
    QLabel* lbl_pwd;
    QLabel* lbl_backupExtFiles;
    QLabel* lbl_fileSHA256;
    QLabel* lbl_closeApp;
    QLabel* lbl_syncDropbox;
    QLabel* lbl_deleteArchives;
    QLabel* lbl_lastNrDay;
    // QLabel* lbl_syncGoogleDrive;

    SwitchButton* btn_setArchivePassword;
    LineEditPassword* edit_pwd;
    SwitchButton* btn_backupExtFiles;
    SwitchButton* btn_createFileSHA256;
    SwitchButton* btn_closeApp;
    SwitchButton* btn_syncDropbox;
    SwitchButton* btn_syncGoogleDrive;
    SwitchButton* btn_deleteArchives;

    QLineEdit* last_nr_day;

    void setupUI();
    void updateUI();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // APPSETTINGS_H
