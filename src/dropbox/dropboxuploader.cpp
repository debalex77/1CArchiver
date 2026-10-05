#include "dropboxuploader.h"
#include "dropboxoauth2_pkce.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSettings>

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

    // Deschidem fișierul
    m_file.setFileName(m_localPath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit uploadFinished(false, "Cannot open source file for upload.");
        return;
    }

    // Construim cererea HTTP
    QNetworkRequest req(QUrl("https://content.dropboxapi.com/2/files/upload"));
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());
    req.setRawHeader("Content-Type", "application/octet-stream");

    // Dropbox-API-Arg
    QJsonObject arg{
        {"path", m_remotePath},
        {"mode", "overwrite"},
        {"autorename", false},
        {"mute", false}
    };

    QByteArray argJson = QJsonDocument(arg).toJson(QJsonDocument::Compact);
    req.setRawHeader("Dropbox-API-Arg", argJson);

    // POST
    m_reply = m_net.post(req, &m_file);

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

    const QString errorMessage = QString::fromUtf8(response);

    // AUTH ERROR (corect, robust)
    if (err == QNetworkReply::AuthenticationRequiredError ||
        err == QNetworkReply::ContentAccessDenied ||
        errorMessage.contains("invalid_access_token") ||
        errorMessage.contains("expired_access_token"))
    {
        if (m_state == UploadState::RefreshingToken)
            return;

        m_state = UploadState::RefreshingToken;
        m_retryAfterRefresh = true;

        emit authError(tr("Dropbox authentication required"));
        tryRefreshToken();
        return;
    }

    // ALTĂ EROARE (închidem fluxul)
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
    emit uploadFinished(false, QString("Refresh token failed: ") + reason);
}
