/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "utils.h"

#include <QByteArray>
#include <QCryptographicHash>

#include <windows.h>
#include <dpapi.h>

static const QString kDpapiPrefix = QStringLiteral("dpapi:");

/** entropie suplimentara: alta aplicatie a aceluiasi utilizator nu decripteaza direct */
static const QByteArray kEntropy = QByteArrayLiteral("1CArchiver-DPAPI-v1");

// ----------------------------------------------------------------------------
// Formatul vechi (XOR + cheie fixa) - pastrat DOAR pentru citire/migrare
// ----------------------------------------------------------------------------
static QByteArray legacyXor(const QByteArray& data)
{
    const QByteArray key =
        QCryptographicHash::hash("1CArchiver-Secure-Key", QCryptographicHash::Sha256);

    QByteArray out = data;
    for (int i = 0; i < out.size(); ++i)
        out[i] = out[i] ^ key[i % key.size()];
    return out;
}

// ----------------------------------------------------------------------------
// DPAPI
// ----------------------------------------------------------------------------
QString encryptPassword(const QString& pass)
{
    if (pass.isEmpty())
        return QString();

    const QByteArray plain = pass.toUtf8();

    DATA_BLOB in{ DWORD(plain.size()),
                  reinterpret_cast<BYTE*>(const_cast<char*>(plain.constData())) };
    DATA_BLOB entropy{ DWORD(kEntropy.size()),
                       reinterpret_cast<BYTE*>(const_cast<char*>(kEntropy.constData())) };
    DATA_BLOB out{};

    if (!CryptProtectData(&in, L"1CArchiver", &entropy, nullptr, nullptr,
                          CRYPTPROTECT_UI_FORBIDDEN, &out))
        return QString();

    const QByteArray cipher(reinterpret_cast<const char*>(out.pbData), int(out.cbData));
    LocalFree(out.pbData);

    return kDpapiPrefix + QString::fromLatin1(cipher.toBase64());
}

QString decryptPassword(const QString& encoded)
{
    if (encoded.isEmpty())
        return QString();

    /** format vechi (v1.8) */
    if (!encoded.startsWith(kDpapiPrefix)) {
        const QByteArray decoded = QByteArray::fromBase64(encoded.toUtf8());
        return QString::fromUtf8(legacyXor(decoded));
    }

    const QByteArray cipher =
        QByteArray::fromBase64(encoded.mid(kDpapiPrefix.size()).toLatin1());

    DATA_BLOB in{ DWORD(cipher.size()),
                  reinterpret_cast<BYTE*>(const_cast<char*>(cipher.constData())) };
    DATA_BLOB entropy{ DWORD(kEntropy.size()),
                       reinterpret_cast<BYTE*>(const_cast<char*>(kEntropy.constData())) };
    DATA_BLOB out{};

    /** esueaza pe alt PC / alt utilizator Windows */
    if (!CryptUnprotectData(&in, nullptr, &entropy, nullptr, nullptr,
                            CRYPTPROTECT_UI_FORBIDDEN, &out))
        return QString();

    const QString plain =
        QString::fromUtf8(reinterpret_cast<const char*>(out.pbData), int(out.cbData));

    SecureZeroMemory(out.pbData, out.cbData);
    LocalFree(out.pbData);

    return plain;
}

bool isLegacyPassword(const QString& encoded)
{
    return !encoded.isEmpty() && !encoded.startsWith(kDpapiPrefix);
}
