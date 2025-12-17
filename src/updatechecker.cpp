#include "updatechecker.h"
#include "src/version.h"

#include <QVersionNumber>

static const char *VERSION_URL =
    "https://raw.githubusercontent.com/debalex77/1CArchiver/master/version.txt";

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject{parent}
{
    connect(&m_net, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onVersionReply);
}

void UpdateChecker::checkForUpdates(const QString &currentVersion)
{
    m_currentVersion = currentVersion.trimmed();
    m_net.get(QNetworkRequest(QUrl(VERSION_URL)));
}

void UpdateChecker::onVersionReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    const QString remoteVersion =
        QString::fromUtf8(reply->readAll()).trimmed();

    reply->deleteLater();

    QVersionNumber vRemote = QVersionNumber::fromString(remoteVersion);
    QVersionNumber vLocal  = QVersionNumber::fromString(m_currentVersion);

    if (!vRemote.isNormalized() || !vLocal.isNormalized()) {
        emit error(tr("Versiune invalidă"));
        return;
    }

    if (QVersionNumber::compare(vRemote, vLocal) > 0) {
        emit updateAvailable(remoteVersion);
    } else {
        emit noUpdate();
    }
}
