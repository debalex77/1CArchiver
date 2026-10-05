#include "dropboxuploader.h"
#include "dropboxoauth2_pkce.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSettings>

/** /files/upload accepta max. 150 MB -> peste prag folosim upload session */
static constexpr qint64 kSimpleUploadLimit = 128LL * 1024 * 1024;
static constexpr qint64 kSessionChunkSize  = 8LL * 1024 * 1024;

/*
 * Dropbox-API-Arg trebuie sa fie ASCII:
 * caracterele non-ASCII (ex. denumiri BD in chirilica) -> \uXXXX
 */
static QByteArray toApiArgHeader(const QJsonObject &arg)
{
    const QString json = QString::fromUtf8(
        QJsonDocument(arg).toJson(QJsonDocument::Compact));

    QByteArray out;
    out.reserve(json.size());
    for (const QChar ch : json) {
        const ushort u = ch.unicode();
        if (u < 0x80)
            out.append(char(u));
        else
            out.append(QStringLiteral("\\u%1")
                           .arg(u, 4, 16, QLatin1Char('0')).toLatin1());
    }
    return out;
}

/*
 * Constructor: stocăm tokenurile primite
 */
DropboxUploader::DropboxUploader(const QString &accessToken,
                                 const QString &refreshToken,
                                 QObject *parent)
    : QObject(parent),
    m_accessToken(accessToken),
    m_refreshToken(refreshToken)
{
}

/*
 * Pornește uploadul cu calea locală + remote
 */
void DropboxUploader::uploadFile(const QString &localPath, const QString &remotePath)
{
    m_localPath  = localPath;
    m_remotePath = remotePath;
    m_refreshAttempted = false;

    m_sessionId.clear();
    m_offset    = 0;
    m_fileSize  = 0;
    m_chunkSize = 0;
    m_finishing = false;

    startUpload();
}

/*
 * Abortam uploadul
 */
void DropboxUploader::abort()
{
    if (m_reply) {
        disconnect(m_reply, nullptr, this, nullptr);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (m_file.isOpen())
        m_file.close();

    m_state = UploadState::Failed;
}

/*
 * Pornim upload
 * Citește fișierul și trimite POST către Dropbox
 */
void DropboxUploader::startUpload()
{
    m_state = UploadState::Uploading;

    // Fisiere mari -> upload session (continua de la m_offset si dupa refresh)
    if (QFileInfo(m_localPath).size() > kSimpleUploadLimit) {
        sendSessionChunk();
        return;
    }

    // Deschidem fișierul
    m_file.setFileName(m_localPath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit uploadFinished(false, "Cannot open source file for upload.");
        return;
    }

    // Construim cererea HTTP
    const QJsonObject arg{
        {"path", m_remotePath},
        {"mode", "overwrite"},
        {"autorename", false},
        {"mute", false}
    };

    // POST
    m_reply = m_net.post(contentRequest("files/upload", arg), &m_file);

    connect(m_reply, &QNetworkReply::uploadProgress,
            this, &DropboxUploader::onUploadProgress);

    connect(m_reply, &QNetworkReply::finished,
            this, &DropboxUploader::onUploadReply);
}

/*
 * Upload progress
 */
void DropboxUploader::onUploadProgress(qint64 sent, qint64 total)
{
    if (m_state != UploadState::Uploading)
        return;

    if (total <= 0 || sent < 0 || sent > total)
        return;

    emit uploadProgress(sent, total);
}

/*
 * Răspuns la upload
 */
void DropboxUploader::onUploadReply()
{
    if (!m_reply)
        return;

    QByteArray response = m_reply->readAll();
    QNetworkReply::NetworkError err = m_reply->error();

    m_reply->deleteLater();
    m_reply = nullptr;

    m_file.close();

    // SUCCESS
    if (err == QNetworkReply::NoError) {
        m_state = UploadState::Idle;
        emit uploadFinished(true, QString::fromUtf8(response));
        return;
    }

    handleReplyError(err, QString::fromUtf8(response));
}

/*
 * Eroare la upload (simplu sau session):
 *  - token expirat -> un singur refresh + retry
 *  - altfel        -> închidem fluxul
 */
void DropboxUploader::handleReplyError(QNetworkReply::NetworkError err,
                                       const QString &errorMessage)
{
    // AUTH ERROR (corect, robust)
    if (err == QNetworkReply::AuthenticationRequiredError ||
        err == QNetworkReply::ContentAccessDenied ||
        errorMessage.contains("invalid_access_token") ||
        errorMessage.contains("expired_access_token"))
    {
        if (m_state == UploadState::RefreshingToken)
            return;

        /** tokenul nou a fost respins si el -> inchidem fluxul */
        if (m_refreshAttempted) {
            m_file.close();
            m_state = UploadState::Failed;
            emit authError(tr("Dropbox authentication required"));
            emit uploadFinished(false, errorMessage);
            return;
        }

        /** starea RefreshingToken o seteaza tryRefreshToken() */
        m_refreshAttempted  = true;
        m_retryAfterRefresh = true;

        tryRefreshToken();
        return;
    }

    // ALTĂ EROARE (închidem fluxul)
    m_file.close();
    m_state = UploadState::Failed;

    emit uploadFinished(
        false,
        errorMessage.isEmpty()
            ? tr("Dropbox upload failed: authentication or network error")
            : errorMessage
        );
}

/*
 * Pornește refresh_token cu PKCE
 */
void DropboxUploader::tryRefreshToken()
{
    if (m_refreshToken.isEmpty()) {
        m_file.close();
        m_state = UploadState::Failed;
        emit uploadFinished(false, "Upload failed: no refresh_token available.");
        return;
    }

    // prevenim refresh paralel
    if (m_state == UploadState::RefreshingToken)
        return;

    m_state = UploadState::RefreshingToken;

    if (!m_oauth) {
        m_oauth = new DropboxOAuth2_PKCE(this);

        connect(m_oauth, &DropboxOAuth2_PKCE::refreshSucceeded,
                this, &DropboxUploader::onRefreshSuccess);

        connect(m_oauth, &DropboxOAuth2_PKCE::refreshFailed,
                this, &DropboxUploader::onRefreshFail);
    }

    m_oauth->refreshAccessToken();
}

/*
 * Refresh OK
 */
void DropboxUploader::onRefreshSuccess()
{
    QSettings s("Oxvalprim", "1CArchiver");
    m_accessToken = s.value("dropbox/access_token").toString();

    if (m_accessToken.isEmpty()) {
        m_file.close();
        m_state = UploadState::Failed;
        emit uploadFinished(false, "Refresh succeeded but access_token missing.");
        return;
    }

    if (!m_retryAfterRefresh) {
        // refresh reușit, dar upload deja abandonat
        m_state = UploadState::Idle;
        return;
    }

    m_retryAfterRefresh = false;
    m_state = UploadState::Idle;

    startUpload();
}

/*
 * Refresh FAIL
 */
void DropboxUploader::onRefreshFail(const QString &reason)
{
    m_file.close();
    m_state = UploadState::Failed;
    emit authError(tr("Dropbox authentication required"));
    emit uploadFinished(false, QString("Refresh token failed: ") + reason);
}

/*
 * Cerere catre content.dropboxapi.com cu Dropbox-API-Arg (ASCII)
 */
QNetworkRequest DropboxUploader::contentRequest(const QString &endpoint,
                                                const QJsonObject &arg) const
{
    QNetworkRequest req(QUrl("https://content.dropboxapi.com/2/" + endpoint));
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());
    req.setRawHeader("Content-Type", "application/octet-stream");
    req.setRawHeader("Dropbox-API-Arg", toApiArgHeader(arg));
    return req;
}

/*
 * Upload session: start -> append_v2 ... -> finish
 * Trimite bucata de la m_offset (la retry dupa refresh - aceeasi bucata)
 */
void DropboxUploader::sendSessionChunk()
{
    if (!m_file.isOpen()) {
        m_file.setFileName(m_localPath);
        if (!m_file.open(QIODevice::ReadOnly)) {
            m_state = UploadState::Failed;
            emit uploadFinished(false, "Cannot open source file for upload.");
            return;
        }
    }

    m_fileSize = m_file.size();

    if (!m_file.seek(m_offset)) {
        m_file.close();
        m_state = UploadState::Failed;
        emit uploadFinished(false, "Cannot read source file for upload.");
        return;
    }

    const QByteArray chunk =
        m_file.read(qMin(kSessionChunkSize, m_fileSize - m_offset));
    m_chunkSize = chunk.size();

    QString endpoint;
    QJsonObject arg;

    if (m_sessionId.isEmpty()) {
        endpoint = "files/upload_session/start";
        arg = QJsonObject{ {"close", false} };
        m_finishing = false;
    } else {
        const QJsonObject cursor{
            {"session_id", m_sessionId},
            {"offset", m_offset}
        };

        m_finishing = (m_offset + m_chunkSize >= m_fileSize);

        if (m_finishing) {
            endpoint = "files/upload_session/finish";
            arg = QJsonObject{
                {"cursor", cursor},
                {"commit", QJsonObject{
                     {"path", m_remotePath},
                     {"mode", "overwrite"},
                     {"autorename", false},
                     {"mute", false}
                 }}
            };
        } else {
            endpoint = "files/upload_session/append_v2";
            arg = QJsonObject{
                {"cursor", cursor},
                {"close", false}
            };
        }
    }

    m_reply = m_net.post(contentRequest(endpoint, arg), chunk);

    /** progres pe tot fisierul, nu pe bucata curenta */
    connect(m_reply, &QNetworkReply::uploadProgress,
            this, [this](qint64 sent, qint64 total) {
                if (m_state != UploadState::Uploading)
                    return;
                if (total <= 0 || sent < 0 || sent > total || m_fileSize <= 0)
                    return;
                emit uploadProgress(m_offset + sent, m_fileSize);
            });

    connect(m_reply, &QNetworkReply::finished,
            this, &DropboxUploader::onSessionReply);
}

/*
 * Răspuns la start / append_v2 / finish
 */
void DropboxUploader::onSessionReply()
{
    if (!m_reply)
        return;

    const QByteArray response = m_reply->readAll();
    const QNetworkReply::NetworkError err = m_reply->error();

    m_reply->deleteLater();
    m_reply = nullptr;

    if (err != QNetworkReply::NoError) {
        handleReplyError(err, QString::fromUtf8(response));
        return;
    }

    if (m_sessionId.isEmpty()) {
        m_sessionId = QJsonDocument::fromJson(response)
                          .object().value("session_id").toString();
        if (m_sessionId.isEmpty()) {
            m_file.close();
            m_state = UploadState::Failed;
            emit uploadFinished(false, "Upload session: missing session_id.");
            return;
        }
    }

    m_offset += m_chunkSize;

    if (m_finishing) {
        m_file.close();
        m_state = UploadState::Idle;
        emit uploadFinished(true, QString::fromUtf8(response));
        return;
    }

    sendSessionChunk();
}
