#ifndef DROPBOXHEALTHCHECKER_H
#define DROPBOXHEALTHCHECKER_H

#include <QNetworkAccessManager>
#include <QObject>

/*
 * Clasa responsabila pentru:
 *   - verificare autorizatiei la Dropbox:
 *     - connected
 *     - authorization required
 */
class DropboxHealthChecker : public QObject
{
    Q_OBJECT
public:
    explicit DropboxHealthChecker(QObject *parent = nullptr);

    void check(const QString &accessToken);

signals:
    void connected();
    void authorizationRequired();

private:
    QNetworkAccessManager m_net;
};

#endif // DROPBOXHEALTHCHECKER_H
