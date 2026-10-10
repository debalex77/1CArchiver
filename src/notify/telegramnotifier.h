/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef TELEGRAMNOTIFIER_H
#define TELEGRAMNOTIFIER_H

#include <QObject>
#include <QNetworkAccessManager>

class QNetworkReply;

/*
 * Trimiterea rezultatului arhivarii in Telegram (Bot API):
 *   - sendMessage  -> rezumatul
 *   - sendDocument -> logul complet (fisier)
 * Config: %APPDATA%/plugins/telegram/telegram.json
 */
class TelegramNotifier : public QObject
{
    Q_OBJECT
public:
    explicit TelegramNotifier(QObject *parent = nullptr);

    static QString configPath();

    bool loadConfig(QString *error);
    bool onlyErrors() const { return m_onlyErrors; }

    /** logData gol -> doar mesajul */
    void send(const QString &text,
              const QByteArray &logData = QByteArray(),
              const QString &logName = QString());

signals:
    void finished(bool ok, const QString &error);

private:
    void postMessage();
    void postDocument();
    bool checkReply(QNetworkReply *reply, QString *error) const;
    QUrl apiUrl(const QString &method) const;
    void finish(bool ok, const QString &error);

private:
    QNetworkAccessManager m_net;

    QString m_token;
    QString m_chatId;
    bool    m_onlyErrors = false;

    QString    m_text;
    QByteArray m_logData;
    QString    m_logName;
    bool       m_done = false;
};

#endif // TELEGRAMNOTIFIER_H
