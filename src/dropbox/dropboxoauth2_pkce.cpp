#include "dropboxoauth2_pkce.h"

#include <QDesktopServices>
#include <QUrlQuery>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QRandomGenerator>

/*
 * Constructor:
 * Încarcă tokenurile din QSettings (dacă există)
 */
DropboxOAuth2_PKCE::DropboxOAuth2_PKCE(QObject *parent)
    : QObject(parent)
{
    QSettings s("Oxvalprim", "1CArchiver");
    m_accessToken  = s.value("dropbox/access_token").toString();
    m_refreshToken = s.value("dropbox/refresh_token").toString();
}

/*
 * Generare code_verifier
 * O secvență aleatorie de 64 de caractere [A-Za-z0-9-._~]
 */
QString DropboxOAuth2_PKCE::generateCodeVerifier()
{
    static const char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

    constexpr int charsCount = sizeof(chars) - 1;

    QString verifier;
    verifier.reserve(64);

    for (int i = 0; i < 64; ++i)
        verifier.append(chars[
            QRandomGenerator::global()->bounded(charsCount)
        ]);

    return verifier;
}

/*
 * Convertire SHA256 → Base64Url
 */
QString DropboxOAuth2_PKCE::base64UrlEncode(const QByteArray &data)
{
    QString out = data.toBase64(QByteArray::Base64UrlEncoding |
                                QByteArray::OmitTrailingEquals);
    return out;
}

/*
 * Generare code_challenge
 * code_challenge = BASE64URL( SHA256(code_verifier) )
 */
QString DropboxOAuth2_PKCE::generateCodeChallenge(const QString &verifier)
{
    QByteArray hash = QCryptographicHash::hash(verifier.toUtf8(),
                                               QCryptographicHash::Sha256);
    return base64UrlEncode(hash);
}

/*
 * Încearcă să pornească serverul HTTP pe unul dintre porturile fallback
 */
bool DropboxOAuth2_PKCE::startCallbackServer()
{
    for (int port : m_ports)
    {
        if (m_server.listen(QHostAddress::LocalHost, port))
        {
            m_callbackPort = port;
            connect(&m_server, &QTcpServer::newConnection,
                    this, &DropboxOAuth2_PKCE::onIncomingConnection);
            return true;
        }
    }
    return false;
}

/*
 * Generează verifier/challenge, pornește serverul local, deschide browser-ul
 */
void DropboxOAuth2_PKCE::startLoginFlow()
{
    m_codeVerifier  = generateCodeVerifier();
    m_codeChallenge = generateCodeChallenge(m_codeVerifier);

    if (!startCallbackServer())
    {
        emit loginFailed("Cannot start local callback server (ports 53682–53686 unavailable)");
        return;
    }

    QUrl url("https://www.dropbox.com/oauth2/authorize");
    QUrlQuery q;
    q.addQueryItem("response_type", "code");
    q.addQueryItem("client_id",     CLIENT_ID);
    q.addQueryItem("redirect_uri",
                   QString("http://localhost:%1/callback").arg(m_callbackPort));
    q.addQueryItem("code_challenge",        m_codeChallenge);
    q.addQueryItem("code_challenge_method", "S256");
    q.addQueryItem("token_access_type",     "offline");

    url.setQuery(q);

    QDesktopServices::openUrl(url);
}

/*
 * Server HTTP local → browser trimite GET /callback?code=xxxx
 */
void DropboxOAuth2_PKCE::onIncomingConnection()
{
    QTcpSocket *sock = m_server.nextPendingConnection();
    if (!sock)
        return;

    sock->waitForReadyRead(3000);
    QString req = sock->readAll();

    // Parsăm codul primit
    QString prefix = "GET /callback?code=";
    int idx = req.indexOf(prefix);
    if (idx < 0) {
        sock->disconnectFromHost();
        emit loginFailed("OAuth redirect invalid – missing authorization code.");
        return;
    }

    QString code = req.mid(idx + prefix.length());
    code = code.left(code.indexOf(' '));

    // Răspuns HTML către browser
    sock->write("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                "<h2>Authentication OK. You may close this window.</h2>");
    sock->disconnectFromHost();

    m_server.close();

    // Schimbăm authorization_code → tokenuri
    exchangeCodeForTokens(code);
}

/*
 * schimbăm codul în tokenuri
 */
void DropboxOAuth2_PKCE::exchangeCodeForTokens(const QString &code)
{
    QUrl url("https://api.dropboxapi.com/oauth2/token");
    QUrlQuery q;

    q.addQueryItem("grant_type", "authorization_code");
    q.addQueryItem("code",       code);
    q.addQueryItem("client_id",  CLIENT_ID);
    q.addQueryItem("redirect_uri",
                   QString("http://localhost:%1/callback").arg(m_callbackPort));
    q.addQueryItem("code_verifier", m_codeVerifier);

    QByteArray body = q.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_net.post(req, body);
    connect(reply, &QNetworkReply::finished,
            this, &DropboxOAuth2_PKCE::onTokenReply);
}

/*
 * Răspuns la schimbul de tokenuri
 */
void DropboxOAuth2_PKCE::onTokenReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonObject obj = QJsonDocument::fromJson(data).object();
    if (obj.contains("error"))
    {
        emit loginFailed(QString::fromUtf8(data));
        return;
    }

    m_accessToken  = obj["access_token"].toString();
    m_refreshToken = obj["refresh_token"].toString();

    // Salvăm tokenurile
    QSettings s("Oxvalprim", "1CArchiver");
    s.setValue("dropbox/access_token",  m_accessToken);
    s.setValue("dropbox/refresh_token", m_refreshToken);

    emit loginSucceeded();
}

/*
 * Refresh access_token
 */
void DropboxOAuth2_PKCE::refreshAccessToken()
{
    if (m_refreshToken.isEmpty()) {
        emit refreshFailed("No refresh_token stored.");
        return;
    }

    QUrl url("https://api.dropboxapi.com/oauth2/token");
    QUrlQuery q;

    q.addQueryItem("grant_type",    "refresh_token");
    q.addQueryItem("refresh_token", m_refreshToken);
    q.addQueryItem("client_id",     CLIENT_ID);

    QByteArray body = q.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_net.post(req, body);
    connect(reply, &QNetworkReply::finished,
            this, &DropboxOAuth2_PKCE::onRefreshReply);
}

/*
 * Răspuns refresh_token
 */
void DropboxOAuth2_PKCE::onRefreshReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonObject obj = QJsonDocument::fromJson(data).object();

    if (obj.contains("error")) {
        emit refreshFailed(QString::fromUtf8(data));
        return;
    }

    m_accessToken = obj["access_token"].toString();

    // Salvăm noul access token
    QSettings s("Oxvalprim", "1CArchiver");
    s.setValue("dropbox/access_token", m_accessToken);

    emit refreshSucceeded();
}

