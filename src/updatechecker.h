#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void checkForUpdates(const QString &currentVersion); /** verificam versiunea noua a app */

signals:
    void updateAvailable(const QString &newVersion); /** versiunea noua este disponibila */
    void noUpdate();
    void error(const QString &msg);

private slots:
    void onVersionReply(QNetworkReply *reply);

private:
    QNetworkAccessManager m_net;
    QString m_currentVersion;

};

#endif // UPDATECHECKER_H
