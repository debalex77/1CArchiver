#pragma once
#include <QString>
#include <QCryptographicHash>

inline QByteArray deriveKey() {
    QByteArray base = "1CArchiver-Secure-Key";     // cheia ta
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

