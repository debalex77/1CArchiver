/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QAbstractButton>
#include <QColor>
#include <QMouseEvent>
#include "globals.h"

class SwitchButton : public QAbstractButton {
    Q_OBJECT
public:
    explicit SwitchButton(QWidget* parent = nullptr);

    QSize sizeHint() const override { return {40, 22}; }
    QColor activeColor;
    QColor inactiveColor;

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

private:
    bool m_checked = false;
};

#endif // SWITCHBUTTON_H
