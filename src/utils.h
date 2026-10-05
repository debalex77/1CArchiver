/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once
#include <QString>
#include <QCryptographicHash>

inline QByteArray deriveKey() {
    QByteArray base = "1CArchiver-Secure-Key"; // La necesitate schimbam -> pu instalarea parolei la rhive
    return QCryptographicHash::hash(base, QCryptographicHash::Sha256);
}

inline QByteArray xorCrypt(const QByteArray& data) {
    QByteArray key = deriveKey();
    QByteArray out = data;
    for (int i = 0; i < out.size(); ++i)
        out[i] = out[i] ^ key[i % key.size()];
    return out;
}

inline QString encryptPassword(const QString& pass) {
    QByteArray encrypted = xorCrypt(pass.toUtf8());
    return encrypted.toBase64();
}

inline QString decryptPassword(const QString& encoded) {
    QByteArray decoded = QByteArray::fromBase64(encoded.toUtf8());
    QByteArray decrypted = xorCrypt(decoded);
    return QString::fromUtf8(decrypted);
}

