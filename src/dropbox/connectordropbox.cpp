#include "connectordropbox.h"
#include "dropboxuploader.h"
#include <QSettings>
#include <QFile>
#include <QDir>

/*
 * Constructor:
 *  Încarcă tokenurile salvate în QSettings
 */
ConnectorDropbox::ConnectorDropbox(QObject *parent)
    : QObject(parent)
{
    QSettings s("Oxvalprim", "1CArchiver");
    m_accessToken  = s.value("dropbox/access_token").toString();
    m_refreshToken = s.value("dropbox/refresh_token").toString();
}

/*
 * Returnează tokenurile actuale
 */
QString ConnectorDropbox::accessToken() const
{
    return m_accessToken;
}

/*
 * Actualizeaza tokenurile
 */
QString ConnectorDropbox::refreshToken() const
{
    return m_refreshToken;
}

/*
 * Salvare tokenuri în QSettings
 */
void ConnectorDropbox::saveTokens(const QString &at, const QString &rt)
{
    m_accessToken  = at;
    m_refreshToken = rt;

    QSettings s("Oxvalprim", "1CArchiver");
    s.setValue("dropbox/access_token",  at);
    s.setValue("dropbox/refresh_token", rt);
    s.sync();
}

/*
 * Test upload – creează fișier temporar și îl trimite la Dropbox
 */
void ConnectorDropbox::testUpload()
{
    if (m_accessToken.isEmpty())
    {
        emit testFinished(false, "No Dropbox access token. Please login.");
        return;
    }

    // Creăm un fișier temporar
    QString tmpFile = QDir::temp().filePath("dropbox_test.txt");

    // Citim fişierul
    QFile f(tmpFile);
    if (!f.open(QIODevice::WriteOnly))
    {
        emit testFinished(false, "Cannot create temporary test file.");
        return;
    }

    f.write("1CArchiver Dropbox Test OK");
    f.close();

    // Uploader
    auto *uploader = new DropboxUploader(m_accessToken, m_refreshToken, this);

    connect(uploader, &DropboxUploader::uploadFinished,
            this, &ConnectorDropbox::onTestFinished);

    uploader->uploadFile(tmpFile, "/dropbox_test.txt");
}

/*
 * Callback pentru test upload
 */
void ConnectorDropbox::onTestFinished(bool ok, const QString &msg)
{
    emit testFinished(ok, msg);
}
