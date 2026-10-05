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
#include <QDebug>

#include <windows.h>
#include <wincrypt.h>

namespace {

const QString kDpapiPrefix = QStringLiteral("dpapi:");

/** entropie suplimentara -> alte aplicatii ale aceluiasi user nu pot decripta "din greseala" */
const QByteArray kEntropy = QByteArrayLiteral("1CArchiver-DPAPI-v1");

//--- formatul vechi (doar pentru citire / migrare)
QByteArray legacyXor(const QByteArray& data)
{
    const QByteArray key = QCryptographicHash::hash("1CArchiver-Secure-Key",
                                                    QCryptographicHash::Sha256);
    QByteArray out = data;
    for (int i = 0; i < out.size(); ++i)
        out[i] = out[i] ^ key[i % key.size()];
    return out;
}

QByteArray dpapiProtect(const QByteArray& plain)
{
    DATA_BLOB in      { DWORD(plain.size()), (BYTE*)plain.constData() };
    DATA_BLOB entropy { DWORD(kEntropy.size()), (BYTE*)kEntropy.constData() };
    DATA_BLOB out     { 0, nullptr };

    if (!CryptProtectData(&in, L"1CArchiver", &entropy, nullptr, nullptr,
                          CRYPTPROTECT_UI_FORBIDDEN, &out)) {
        qWarning() << "CryptProtectData a esuat, cod:" << GetLastError();
        return {};
    }

    QByteArray result(reinterpret_cast<const char*>(out.pbData), int(out.cbData));
    LocalFree(out.pbData);
    return result;
}

bool dpapiUnprotect(const QByteArray& cipher, QByteArray& plain)
{
    DATA_BLOB in      { DWORD(cipher.size()), (BYTE*)cipher.constData() };
    DATA_BLOB entropy { DWORD(kEntropy.size()), (BYTE*)kEntropy.constData() };
    DATA_BLOB out     { 0, nullptr };

    if (!CryptUnprotectData(&in, nullptr, &entropy, nullptr, nullptr,
                            CRYPTPROTECT_UI_FORBIDDEN, &out)) {
        qWarning() << "CryptUnprotectData a esuat, cod:" << GetLastError();
        return false;
    }

    plain = QByteArray(reinterpret_cast<const char*>(out.pbData), int(out.cbData));
    SecureZeroMemory(out.pbData, out.cbData);
    LocalFree(out.pbData);
    return true;
}

} // namespace

QString encryptPassword(const QString& pass)
{
    if (pass.isEmpty())
        return QString();

    const QByteArray cipher = dpapiProtect(pass.toUtf8());
    if (cipher.isEmpty())
        return QString();

    return kDpapiPrefix + QString::fromLatin1(cipher.toBase64());
}

QString decryptPassword(const QString& encoded)
{
    if (encoded.isEmpty())
        return QString();

    if (encoded.startsWith(kDpapiPrefix)) {
        const QByteArray cipher =
            QByteArray::fromBase64(encoded.mid(kDpapiPrefix.size()).toLatin1());
        QByteArray plain;
        if (!dpapiUnprotect(cipher, plain))
            return QString();
        return QString::fromUtf8(plain);
    }

    //--- format vechi (XOR) -> se va re-cripta cu DPAPI la urmatoarea salvare
    return QString::fromUtf8(legacyXor(QByteArray::fromBase64(encoded.toUtf8())));
}
