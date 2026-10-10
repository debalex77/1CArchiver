#include "telegramnotifier.h"
#include "src/utils.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>

static constexpr int kTimeoutMs   = 30000; /** fara retea -> nu blocam --autorun */
static constexpr int kMaxTextSize = 4000;  /** limita Telegram: 4096 caractere */

TelegramNotifier::TelegramNotifier(QObject *parent)
    : QObject(parent)
{
}

QString TelegramNotifier::configPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + "/plugins/telegram/telegram.json";
}

bool TelegramNotifier::loadConfig(QString *error)
{
    QFile f(configPath());
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = tr("Telegram nu este configurat.");
        return false;
    }

    const QJsonObject cfg = QJsonDocument::fromJson(f.readAll()).object();

    /** tokenul se salveaza in campul "password" -> criptat DPAPI de PluginConfigDialog */
    m_token      = decryptPassword(cfg.value("password").toString()).trimmed();
    m_chatId     = cfg.value("chat_id").toString().trimmed();
    m_onlyErrors = cfg.value("mode").toString() == "errors";

    if (m_token.isEmpty() || m_chatId.isEmpty()) {
        if (error)
            *error = tr("Telegram: lipsește Bot token sau Chat ID.");
        return false;
    }
    return true;
}

void TelegramNotifier::send(const QString &text,
                            const QByteArray &logData,
                            const QString &logName)
{
    m_text    = text.left(kMaxTextSize);
    m_logData = logData;
    m_logName = logName;
    m_done    = false;

    postMessage();
}

QUrl TelegramNotifier::apiUrl(const QString &method) const
{
    return QUrl(QString("https://api.telegram.org/bot%1/%2").arg(m_token, method));
}

void TelegramNotifier::postMessage()
{
    QNetworkRequest req(apiUrl("sendMessage"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setTransferTimeout(kTimeoutMs);

    QJsonObject body;
    body["chat_id"] = m_chatId;
    body["text"]    = m_text;

    QNetworkReply *reply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        QString err;
        if (!checkReply(reply, &err)) {
            finish(false, err);
            return;
        }

        if (m_logData.isEmpty())
            finish(true, QString());
        else
            postDocument();
    });
}

void TelegramNotifier::postDocument()
{
    auto *multi = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart chat;
    chat.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QVariant("form-data; name=\"chat_id\""));
    chat.setBody(m_chatId.toUtf8());
    multi->append(chat);

    QHttpPart doc;
    doc.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain; charset=utf-8"));
    doc.setHeader(QNetworkRequest::ContentDispositionHeader,
                  QVariant(QString("form-data; name=\"document\"; filename=\"%1\"")
                               .arg(m_logName.isEmpty() ? QStringLiteral("1CArchiver.log")
                                                        : m_logName)));
    doc.setBody(m_logData);
    multi->append(doc);

    QNetworkRequest req(apiUrl("sendDocument"));
    req.setTransferTimeout(kTimeoutMs);

    QNetworkReply *reply = m_net.post(req, multi);
    multi->setParent(reply); /** se elibereaza odata cu reply */

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        QString err;
        finish(checkReply(reply, &err), err);
    });
}

bool TelegramNotifier::checkReply(QNetworkReply *reply, QString *error) const
{
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();

    if (reply->error() == QNetworkReply::NoError && obj.value("ok").toBool())
        return true;

    if (error) {
        /** descrierea Telegram (ex. "Unauthorized", "chat not found") */
        QString msg = obj.value("description").toString();
        if (msg.isEmpty())
            msg = reply->errorString();

        /** errorString() contine URL-ul -> ascundem tokenul */
        msg.replace(m_token, "***");
        *error = msg;
    }
    return false;
}

void TelegramNotifier::finish(bool ok, const QString &error)
{
    if (m_done)
        return;
    m_done = true;

    emit finished(ok, error);
}
