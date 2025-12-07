/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "lineeditpassword.h"

LineEditPassword::LineEditPassword(QWidget *parent) : QLineEdit(parent)
{
    action_hideShowPassword = this->addAction(QIcon(":/icons/icons/lock.png"), QLineEdit::ActionPosition::TrailingPosition);
    connect(action_hideShowPassword, &QAction::triggered, this, &LineEditPassword::onClickedButton);

    show_passwd = false;
    this->setEchoMode(QLineEdit::Password);
}

LineEditPassword::~LineEditPassword()
{

}

void LineEditPassword::onClickedButton()
{
    if (show_passwd) {
        action_hideShowPassword->setIcon(QIcon(":/icons/icons/lock.png"));
        show_passwd = false;
        this->setEchoMode(QLineEdit::Password);
    } else {
        action_hideShowPassword->setIcon(QIcon(":/icons/icons/unlock.png"));
        show_passwd = true;
        this->setEchoMode(QLineEdit::Normal);
    }
}
