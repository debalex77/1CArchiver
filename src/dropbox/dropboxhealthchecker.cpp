#include "dropboxhealthchecker.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

DropboxHealthChecker::DropboxHealthChecker(QObject *parent)
    : QObject{parent}
{}

void DropboxHealthChecker::check(const QString &accessToken)
{
    if (accessToken.isEmpty()) {
        emit authorizationRequired();
        deleteLater();
        return;
    }

    QNetworkRequest req(
        QUrl("https://api.dropboxapi.com/2/users/get_current_account"));

    req.setRawHeader("Authorization",
                     "Bearer " + accessToken.toUtf8());

    // req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_net.post(req, QByteArray()); // body COMPLET gol !!!

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]()
            {
                const int httpStatus =
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                reply->deleteLater();

                if (httpStatus == 200)
                    emit connected();
                else
                    emit authorizationRequired();

                deleteLater();
            });
}
