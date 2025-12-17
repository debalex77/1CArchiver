#include "updatedialog.h"

UpdateDialog::UpdateDialog(const QString &version, QWidget *parent)
    : QDialog(parent), m_version(version)
{
    setWindowTitle(tr("Actualizare disponibilă"));
    setModal(true);
    resize(460, 200);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(12);

    /** --- Header (icon + titlu) */
    auto *headerLay = new QHBoxLayout;

    auto *icon = new QLabel;
    icon->setPixmap(QIcon(":/icons/icons/update.png")
                        .pixmap(48, 48));
    headerLay->addWidget(icon);

    auto *titleLay = new QVBoxLayout;
    titleLay->addWidget(new QLabel(
        tr("<b>Este disponibilă o versiune nouă</b>")));
    titleLay->addWidget(new QLabel(
        tr("Versiunea nouă: <b>%1</b>").arg(version)));

    headerLay->addLayout(titleLay);
    headerLay->addStretch();

    mainLay->addLayout(headerLay);

    /** --- Text info */
    auto *info = new QLabel(
        tr("Se recomandă actualizarea aplicației pentru a beneficia de "
           "funcționalități noi și corecții de erori."));
    info->setWordWrap(true);
    info->setStyleSheet("color: gray;");
    mainLay->addWidget(info);

    /** --- Progress */
    m_progress = new QProgressBar;
    m_progress->setRange(0, 100);
    m_progress->setVisible(false);
    mainLay->addWidget(m_progress);

    /** --- Butoane */
    auto *btnLay = new QHBoxLayout;
    btnLay->addStretch();

    auto *btnLater = new QPushButton(tr("Mai târziu"));
    m_btn = new QPushButton(tr("Instalează acum"));
    m_btn->setDefault(true);

    btnLay->addWidget(btnLater);
    btnLay->addWidget(m_btn);

    mainLay->addLayout(btnLay);

    connect(m_btn, &QPushButton::clicked,
            this, &UpdateDialog::startDownload);

    connect(btnLater, &QPushButton::clicked,
            this, &QDialog::reject);
}

void UpdateDialog::startDownload()
{
    m_progress->setVisible(true);
    m_btn->setEnabled(false);

    const QString url =
        QString("https://github.com/debalex77/1CArchiver/releases/download/v%1/1CArchiver_v%1_Windows_amd64.exe")
            .arg(m_version);

    m_reply = m_net.get(QNetworkRequest(QUrl(url)));

    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &UpdateDialog::onDownloadProgress);

    connect(m_reply, &QNetworkReply::finished,
            this, &UpdateDialog::onFinished);
}

void UpdateDialog::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0)
        m_progress->setValue(int(received * 100 / total));
}

void UpdateDialog::onFinished()
{
    const QString path =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
        + QString("/1CArchiver_v%1_Windows_amd64.exe")
              .arg(m_version);

    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(m_reply->readAll());

    f.close();
    m_reply->deleteLater();

    QProcess::startDetached(path);
    qApp->quit();
}
