/*
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2025 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINEEDITPASSWORD_H
#define LINEEDITPASSWORD_H

#include <QLineEdit>

/*
 * Clasa pentru:
 *   - QLineEdit cu functia vizualizarii proprietatii
 *     setEchoMode(QLineEdit::Password)
 */
class LineEditPassword : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEditPassword(QWidget *parent = nullptr);
    ~LineEditPassword();

private slots:
    void onClickedButton();

private:
    bool show_passwd;
    QAction *action_hideShowPassword;
};

#endif // LINEEDITPASSWORD_H
